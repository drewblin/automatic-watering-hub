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
#include "SettingsSnapshot.hpp"

class Settings
{
public:
    Settings();
    void begin();
    SettingsSnapshot get() const;
    bool save(const SettingsSnapshot &snapshot, String &error);
    bool saveWifiSettings(const WifiSettings &wifiSettings, String &error);

private:
    static std::string generateApiAccessToken();
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
    static bool putString(
        Preferences &preferences,
        const char *key,
        const char *value);
    static bool hasRequiredKey(
        Preferences &preferences,
        bool storageAvailable,
        const char *key);

    SettingsSnapshot snapshot_;
};
