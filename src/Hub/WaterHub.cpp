#include "WaterHub.hpp"

void WaterHub::setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    magistralWaterCounter_ = std::move(counter);
}

void WaterHub::setPressureSensor(std::unique_ptr<PressureSendor> sensor)
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

void WaterHub::addValve(std::unique_ptr<Valve> valve, SoilSensor* sensor)
{
    valves_.push_back(std::move(valve));
    valveToSoilSensorMap_[valves_.back().get()] = sensor;
}
