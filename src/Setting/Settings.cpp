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

std::vector<SoilSensorSetting> Settings::getSoilSensorSetting()
{
    return std::vector<SoilSensorSetting>();
}
