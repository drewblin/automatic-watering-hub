#pragma once

#include <optional>
#include <vector>
#include "GlobalSettings.hpp"
#include "PressureSensorSetting.hpp"
#include "SoilSensorSetting.hpp"
#include "ValveSetting.hpp"
#include "WaterCounterSetting.hpp"
#include "WifiSettings.hpp"

struct SettingsSnapshot
{
    GlobalSettings globalSettings;
    WifiSettings wifiSettings;
    std::vector<ValveSetting> valveSettings;
    std::optional<PressureSensorSetting> pressureSensorSetting;
    std::optional<WaterCounterSetting> magistralWaterCounterSetting;
    std::vector<WaterCounterSetting> leafWaterCounterSettings;
    std::vector<SoilSensorSetting> soilSensorSettings;

    bool hasRequiredWaterHubSettings() const
    {
        return hasPressureSensorSetting() && hasMagistralWaterCounterSetting();
    }

    bool hasPressureSensorSetting() const
    {
        return pressureSensorSetting.has_value();
    }

    bool hasMagistralWaterCounterSetting() const
    {
        return magistralWaterCounterSetting.has_value();
    }
};
