#pragma once

#include <Preferences.h>
#include <optional>
#include <vector>
#include "ValveSetting.hpp"
#include "PressureSensorSetting.hpp"
#include "WaterCounterSetting.hpp"
#include "SoilSensorSetting.hpp"
#include "GlobalSettings.hpp"
#include "WifiSettings.hpp"

class Settings
{
public:
    Settings();
    void begin();
    bool hasRequiredWaterHubSettings() const;
    GlobalSettings getGlobalSettings();
    WifiSettings getWifiSettings();
    std::vector<ValveSetting> getValveSetting();
    PressureSensorSetting getPressureSensorSetting();
    WaterCounterSetting getMagistralWaterCounterSetting();
    std::vector<WaterCounterSetting> getLeafWaterCounterSetting();
    std::vector<SoilSensorSetting> getSoilSensorSetting();

private:
    static uint32_t readUInt(
        Preferences &preferences,
        bool storageAvailable,
        const char *key,
        uint32_t defaultValue);
    static uint8_t readUChar(
        Preferences &preferences,
        bool storageAvailable,
        const char *key,
        uint8_t defaultValue);
    static uint8_t readCount(
        Preferences &preferences,
        bool storageAvailable,
        const char *key);
    static float readFloat(
        Preferences &preferences,
        bool storageAvailable,
        const char *key,
        float defaultValue);
    static String readString(
        Preferences &preferences,
        bool storageAvailable,
        const char *key,
        const String &defaultValue);
    static bool hasRequiredKey(
        Preferences &preferences,
        bool storageAvailable,
        const char *key);

    GlobalSettings globalSettings_;
    WifiSettings wifiSettings_;
    std::vector<ValveSetting> valveSettings_;
    std::optional<PressureSensorSetting> pressureSensorSetting_;
    std::optional<WaterCounterSetting> magistralWaterCounterSetting_;
    std::vector<WaterCounterSetting> leafWaterCounterSettings_;
    std::vector<SoilSensorSetting> soilSensorSettings_;
};
