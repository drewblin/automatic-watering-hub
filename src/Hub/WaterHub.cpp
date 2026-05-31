#include "WaterHub.hpp"

WaterHub::WaterHub(GlobalSettings globalSettings) : globalSettings_(globalSettings)
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
    readPressureSensor();
    readSoilSensors();
    readWaterCounters();
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

void WaterHub::readWaterCounters()
{
    uint32_t currentTimeMs = millis();
    uint32_t waterCounterReadIntervalMs =
        globalSettings_.waterCounterReadIntervalSeconds * 1000;

    if (currentTimeMs - lastWaterCounterReadTimeMs_ < waterCounterReadIntervalMs)
    {
        return;
    }

    lastWaterCounterReadTimeMs_ = currentTimeMs;

    magistralWaterCounter_->readLiters();

    for (const auto &counter : leafWaterCounters_)
    {
        counter->readLiters();
    }
}

void WaterHub::readPressureSensor()
{
    uint32_t currentTimeMs = millis();
    uint32_t pressureSensorReadIntervalMs =
        globalSettings_.pressureSensorReadIntervalSeconds * 1000;

    if (currentTimeMs - lastPressureSensorReadTimeMs_ < pressureSensorReadIntervalMs)
    {
        return;
    }

    lastPressureSensorReadTimeMs_ = currentTimeMs;

    pressureSensor_->readPressure();
}

void WaterHub::readSoilSensors()
{
    uint32_t currentTimeMs = millis();
    uint32_t soilSensorReadIntervalMs =
        globalSettings_.soilSensorReadIntervalSeconds * 1000;

    if (currentTimeMs - lastSoilSensorReadTimeMs_ < soilSensorReadIntervalMs)
    {
        return;
    }

    lastSoilSensorReadTimeMs_ = currentTimeMs;

    for (size_t i = 0; i < soilSensors_.size(); ++i)
    {
        soilSensors_[i]->readData();
    }
}
