#include "WaterHub.hpp"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

namespace
{
SemaphoreHandle_t valveMutex()
{
    static SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    configASSERT(mutex != nullptr);
    return mutex;
}

class ValveStateLock
{
public:
    ValveStateLock() { xSemaphoreTake(valveMutex(), portMAX_DELAY); }
    ~ValveStateLock() { xSemaphoreGive(valveMutex()); }
};
}

WaterHub::WaterHub(const GlobalSettings &globalSettings, ModbusSensorPower &modbusSensorPower)
    : globalSettings_(globalSettings), modbusSensorPower_(modbusSensorPower)
{
}

void WaterHub::setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    magistralWaterCounter_ = std::move(counter);
}

void WaterHub::setPressureSensor(std::unique_ptr<PressureSensor> sensor)
{
    pressureSensor_ = std::move(sensor);
}

void WaterHub::addLeafWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    leafWaterCounters_.push_back(std::move(counter));
}

void WaterHub::addSoilSensor(std::unique_ptr<SoilSensor> sensor)
{
    soilSensors_.push_back(std::move(sensor));
}

void WaterHub::addValve(std::unique_ptr<Valve> valve, SoilSensor *sensor)
{
    valves_.push_back(std::move(valve));
    valveToSoilSensorMap_[valves_.back().get()] = sensor;
}

void WaterHub::loop()
{
    const uint32_t currentTimeMs = millis();

    syncWateringPower();
    updateSensorReadIntervals();
    readWaterCounters(currentTimeMs);

    modbusSensorPower_.setScheduledReadActive(hasDueModbusSensor(currentTimeMs));
    readPressureSensor(currentTimeMs);
    readSoilSensors(currentTimeMs);
}

const WaterCounter *WaterHub::getMagistralWaterCounter() const
{
    return magistralWaterCounter_.get();
}

const PressureSensor *WaterHub::getPressureSensor() const
{
    return pressureSensor_.get();
}

const std::vector<std::unique_ptr<WaterCounter>> &WaterHub::getLeafWaterCounters() const
{
    return leafWaterCounters_;
}

const std::vector<std::unique_ptr<SoilSensor>> &WaterHub::getSoilSensors() const
{
    return soilSensors_;
}

const std::vector<std::unique_ptr<Valve>> &WaterHub::getValves() const
{
    return valves_;
}

SoilSensor *WaterHub::getSoilSensorForValve(const Valve *valve) const
{
    auto it = valveToSoilSensorMap_.find(valve);
    if (it == valveToSoilSensorMap_.end())
    {
        return nullptr;
    }

    return it->second;
}

void WaterHub::updateSensorReadIntervals()
{
    const bool shouldUseWateringMode = hasOpenValve();
    if (sensorReadIntervalsUseWateringMode_ == shouldUseWateringMode)
    {
        return;
    }

    sensorReadIntervalsUseWateringMode_ = shouldUseWateringMode;

    const uint32_t waterCounterReadIntervalSeconds = shouldUseWateringMode
                                                         ? globalSettings_.wateringWaterCounterReadIntervalSeconds
                                                         : globalSettings_.idleWaterCounterReadIntervalSeconds;
    const uint32_t pressureSensorReadIntervalSeconds = shouldUseWateringMode
                                                           ? globalSettings_.wateringPressureSensorReadIntervalSeconds
                                                           : globalSettings_.idlePressureSensorReadIntervalSeconds;
    const uint32_t soilSensorReadIntervalSeconds = shouldUseWateringMode
                                                       ? globalSettings_.wateringSoilSensorReadIntervalSeconds
                                                       : globalSettings_.idleSoilSensorReadIntervalSeconds;

    if (magistralWaterCounter_ != nullptr)
    {
        magistralWaterCounter_->setReadIntervalSeconds(waterCounterReadIntervalSeconds);
    }

    for (const auto &counter : leafWaterCounters_)
    {
        counter->setReadIntervalSeconds(waterCounterReadIntervalSeconds);
    }

    if (pressureSensor_ != nullptr)
    {
        pressureSensor_->setReadIntervalSeconds(pressureSensorReadIntervalSeconds);
    }

    for (const auto &sensor : soilSensors_)
    {
        sensor->setReadIntervalSeconds(soilSensorReadIntervalSeconds);
    }
}

void WaterHub::readWaterCounters(uint32_t currentTimeMs)
{
    if (magistralWaterCounter_ != nullptr)
    {
        magistralWaterCounter_->readLitersIfDue(currentTimeMs);
    }

    for (const auto &counter : leafWaterCounters_)
    {
        counter->readLitersIfDue(currentTimeMs);
    }
}

void WaterHub::readPressureSensor(uint32_t currentTimeMs)
{
    if (pressureSensor_ != nullptr &&
        modbusSensorPower_.isReady())
    {
        pressureSensor_->readPressureIfDue(currentTimeMs);
    }
}

void WaterHub::readSoilSensors(uint32_t currentTimeMs)
{
    if (!modbusSensorPower_.isReady())
    {
        return;
    }

    for (const auto &sensor : soilSensors_)
    {
        sensor->readDataIfDue(currentTimeMs);
    }
}

bool WaterHub::hasOpenValve() const
{
    ValveStateLock lock;
    return hasOpenValveUnlocked();
}

bool WaterHub::hasOpenValveUnlocked() const
{
    for (const auto &valve : valves_)
    {
        if (valve->isOpen())
        {
            return true;
        }
    }

    return false;
}

void WaterHub::syncWateringPower()
{
    ValveStateLock lock;
    modbusSensorPower_.setWateringActive(hasOpenValveUnlocked());
}

bool WaterHub::hasDueModbusSensor(uint32_t currentTimeMs) const
{
    if (pressureSensor_ != nullptr && pressureSensor_->isReadDue(currentTimeMs))
    {
        return true;
    }
    for (const auto &sensor : soilSensors_)
    {
        if (sensor->isReadDue(currentTimeMs))
        {
            return true;
        }
    }
    return false;
}

void WaterHub::openValve(Valve &valve)
{
    ValveStateLock lock;
    modbusSensorPower_.setWateringActive(true);
    valve.open();
}

void WaterHub::openValveForTime(Valve &valve, uint32_t seconds)
{
    ValveStateLock lock;
    modbusSensorPower_.setWateringActive(true);
    valve.openForTime(seconds);
}

void WaterHub::closeValve(Valve &valve)
{
    ValveStateLock lock;
    valve.close();
}

void WaterHub::closeExpiredValves(uint32_t currentTimeMs)
{
    ValveStateLock lock;
    for (const auto &valve : valves_)
    {
        if (valve->isOpenExpired(currentTimeMs))
        {
            valve->close();
        }
    }
}

void WaterHub::closeAllValves()
{
    ValveStateLock lock;
    for (const auto &valve : valves_)
    {
        valve->close();
    }
}
