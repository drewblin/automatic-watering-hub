#include "WaterCounterSetting.hpp"

WaterCounterSetting::WaterCounterSetting(uint8_t pin, std::string name, float litersPerTick)
{
}

uint8_t WaterCounterSetting::getPin()
{
    return 0;
}

float WaterCounterSetting::getLitersPerTick()
{
    return 1.0f;
}

std::string WaterCounterSetting::getName()
{
    return "";
}