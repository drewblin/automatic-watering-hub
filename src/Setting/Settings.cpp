#include "Settings.hpp"

Settings::Settings()
{
}

std::vector<ValveSetting> Settings::getValveSetting()
{
    return std::vector<ValveSetting>();
}

PresureSensorSetting Settings::getPresureSensorSetting()
{
    return PresureSensorSetting();
}

WaterCounterSetting Settings::getMagistralWaterCounterSetting()
{
    return WaterCounterSetting();
}

std::vector<WaterCounterSetting> Settings::getLeafWaterCounterSetting()
{
    return std::vector<WaterCounterSetting>();
}

std::vector<HumiditySensorSetting> Settings::getHumiditySensorSetting()
{
    return std::vector<HumiditySensorSetting>();
}
