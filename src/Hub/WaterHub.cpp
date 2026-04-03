#include "WaterHub.hpp"

void WaterHub::setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    magistralWaterCounter = std::move(counter);
}

void WaterHub::setPresureSensor(std::unique_ptr<PresureSendor> sensor)
{
    presureSensor = std::move(sensor);
}

void WaterHub::addLeafWaterCounter(std::unique_ptr<WaterCounter> counter)
{
    leafWaterCounters.push_back(std::move(counter));
}

void WaterHub::addSoilSensor(std::unique_ptr<SoilSensor> sensor)
{
    soilSensors.push_back(std::move(sensor));
}

void WaterHub::addValve(std::unique_ptr<Valve> valve, SoilSensor* sensor)
{
    valves.push_back(std::move(valve));
    valveToSoilSensorMap[valves.back().get()] = sensor;
}
