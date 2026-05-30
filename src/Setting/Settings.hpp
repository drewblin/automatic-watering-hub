#pragma once

#include <vector>
#include "ArduinoJson.h"
#include "ValveSetting.hpp"
#include "PressureSensorSetting.hpp"
#include "WaterCounterSetting.hpp"
#include "SoilSensorSetting.hpp"
#include "GlobalSettings.hpp"

class Settings
{
public:
    Settings();
    GlobalSettings getGlobalSettings();
    std::vector<ValveSetting> getValveSetting();
    PressureSensorSetting getPressureSensorSetting();
    WaterCounterSetting getMagistralWaterCounterSetting();
    std::vector<WaterCounterSetting> getLeafWaterCounterSetting();
    std::vector<SoilSensorSetting> getSoilSensorSetting();

private:
    JsonDocument doc_;
};
