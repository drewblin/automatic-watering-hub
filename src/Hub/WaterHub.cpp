#include "WaterHub.hpp"

WaterHub::WaterHub(const GlobalSettings &globalSettings)
    : globalSettings_(globalSettings)
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
    uint32_t currentTimeMs = millis();

    updateSensorReadIntervals();
    readPressureSensor(currentTimeMs);
    readSoilSensors(currentTimeMs);
    readWaterCounters(currentTimeMs);
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
    if (pressureSensor_ != nullptr)
    {
        pressureSensor_->readPressureIfDue(currentTimeMs);
    }
}

void WaterHub::readSoilSensors(uint32_t currentTimeMs)
{
    for (size_t i = 0; i < soilSensors_.size(); ++i)
    {
        soilSensors_[i]->readDataIfDue(currentTimeMs);
    }
}

bool WaterHub::hasOpenValve() const
{
    for (const auto &valve : getValves())
    {
        if (valve->isOpen())
        {
            return true;
        }
    }

    return false;
}
