#include "WaterCounterSetting.hpp"

WaterCounterSetting::WaterCounterSetting(uint8_t pin, std::string name, float litersPerTick)
    : pin_(pin), name_(name), litersPerTick_(litersPerTick)
{
}

uint8_t WaterCounterSetting::getPin()
{
    return pin_;
}

float WaterCounterSetting::getLitersPerTick()
{
    return litersPerTick_;
}

std::string WaterCounterSetting::getName()
{
    return name_;
}