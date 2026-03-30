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

void WaterHub::addHumiditySensor(std::unique_ptr<HumiditySensor> sensor)
{
    humiditySensors.push_back(std::move(sensor));
}

void WaterHub::addValve(std::unique_ptr<Valve> valve, HumiditySensor* sensor)
{
    valves.push_back(std::move(valve));
    valveToHumiditySensorMap[valves.back().get()] = sensor;
}
