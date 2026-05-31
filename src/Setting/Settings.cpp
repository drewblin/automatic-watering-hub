#include "Settings.hpp"

Settings::Settings()
{
}

void Settings::begin()
{
    Preferences preferences;
    bool storageAvailable = preferences.begin("watering", true);
    if (!storageAvailable)
    {
        ESP_LOGE("Settings", "Failed to open NVS namespace. Using hardcoded defaults where available");
    }

    globalSettings_.idleWaterCounterReadIntervalSeconds = readUInt(
        preferences, storageAvailable, "iWcRead", 5 * 60);
    globalSettings_.wateringWaterCounterReadIntervalSeconds = readUInt(
        preferences, storageAvailable, "wWcRead", 1 * 60);
    globalSettings_.idlePressureSensorReadIntervalSeconds = readUInt(
        preferences, storageAvailable, "iPrRead", 5 * 60);
    globalSettings_.wateringPressureSensorReadIntervalSeconds = readUInt(
        preferences, storageAvailable, "wPrRead", 5);
    globalSettings_.idleSoilSensorReadIntervalSeconds = readUInt(
        preferences, storageAvailable, "iSoilRead", 60 * 60);
    globalSettings_.wateringSoilSensorReadIntervalSeconds = readUInt(
        preferences, storageAvailable, "wSoilRead", 1 * 60);
    globalSettings_.maximumManualValveOpenTimeSeconds = readUInt(
        preferences, storageAvailable, "maxManual", 3600);
    globalSettings_.startWateringBelowHumidityPercent = readUChar(
        preferences, storageAvailable, "humidStart", 20);
    globalSettings_.stopWateringAboveHumidityPercent = readUChar(
        preferences, storageAvailable, "humidStop", 80);

    TimeOfDay scheduledWateringStartTime{
        readUChar(preferences, storageAvailable, "startHour", 23),
        readUChar(preferences, storageAvailable, "startMinute", 10)};
    String wateringStartModeValue = readString(
        preferences, storageAvailable, "waterMode", "atScheduledTime");
    std::optional<WateringStartMode> wateringStartMode = WateringStartMode::tryFrom(
        wateringStartModeValue.c_str(),
        scheduledWateringStartTime);
    if (!wateringStartMode.has_value())
    {
        ESP_LOGE("Settings", "Invalid setting waterMode. Using hardcoded default");
        wateringStartMode = WateringStartMode::immediately();
    }
    globalSettings_.wateringStartMode = wateringStartMode.value();

    globalSettings_.zoneWateringDurationSeconds = readUInt(
        preferences, storageAvailable, "zoneDuration", 5 * 60);
    globalSettings_.zoneWateringRetryDelaySeconds = readUInt(
        preferences, storageAvailable, "zoneRetry", 15 * 60);

    wifiSettings_.ssid = readString(preferences, storageAvailable, "wifiSsid", "Lypky").c_str();
    wifiSettings_.password = readString(preferences, storageAvailable, "wifiPassword", "79348454").c_str();

    uint8_t valveCount = readCount(preferences, storageAvailable, "valveCount");
    valveSettings_.clear();
    valveSettings_.reserve(valveCount);
    for (uint8_t i = 0; i < valveCount; ++i)
    {
        String prefix = "v" + String(i);
        valveSettings_.emplace_back(
            readUChar(preferences, storageAvailable, (prefix + "Pin").c_str(), 19),
            readString(preferences, storageAvailable, (prefix + "Name").c_str(), "").c_str(),
            readUChar(preferences, storageAvailable, (prefix + "Soil").c_str(), 1));
    }

    bool hasPressureSensorSetting = hasRequiredKey(preferences, storageAvailable, "pressureAddr");
    pressureSensorSetting_.reset();
    if (hasPressureSensorSetting)
    {
        pressureSensorSetting_.emplace(preferences.getUChar("pressureAddr"));
    }

    bool hasMagistralWaterCounterPin =
        hasRequiredKey(preferences, storageAvailable, "mainWcPin");
    bool hasMagistralWaterCounterName =
        hasRequiredKey(preferences, storageAvailable, "mainWcName");
    bool hasMagistralWaterCounterLitersPerTick =
        hasRequiredKey(preferences, storageAvailable, "mainWcLpt");
    bool hasMagistralWaterCounterSetting =
        hasMagistralWaterCounterPin &&
        hasMagistralWaterCounterName &&
        hasMagistralWaterCounterLitersPerTick;
    magistralWaterCounterSetting_.reset();
    if (hasMagistralWaterCounterSetting)
    {
        magistralWaterCounterSetting_.emplace(
            preferences.getUChar("mainWcPin"),
            preferences.getString("mainWcName").c_str(),
            preferences.getFloat("mainWcLpt"));
    }

    uint8_t leafWaterCounterCount = readCount(preferences, storageAvailable, "leafWcCount");
    leafWaterCounterSettings_.clear();
    leafWaterCounterSettings_.reserve(leafWaterCounterCount);
    for (uint8_t i = 0; i < leafWaterCounterCount; ++i)
    {
        String prefix = "l" + String(i);
        leafWaterCounterSettings_.emplace_back(
            readUChar(preferences, storageAvailable, (prefix + "Pin").c_str(), 36),
            readString(preferences, storageAvailable, (prefix + "Name").c_str(), "").c_str(),
            readFloat(preferences, storageAvailable, (prefix + "Lpt").c_str(), 1));
    }

    uint8_t soilSensorCount = readCount(preferences, storageAvailable, "soilCount");
    soilSensorSettings_.clear();
    soilSensorSettings_.reserve(soilSensorCount);
    for (uint8_t i = 0; i < soilSensorCount; ++i)
    {
        String prefix = "s" + String(i);
        soilSensorSettings_.emplace_back(
            readUChar(preferences, storageAvailable, (prefix + "Addr").c_str(), 1),
            readString(preferences, storageAvailable, (prefix + "Name").c_str(), "").c_str());
    }

    if (storageAvailable)
    {
        preferences.end();
    }
}

bool Settings::hasRequiredWaterHubSettings() const
{
    return pressureSensorSetting_.has_value() && magistralWaterCounterSetting_.has_value();
}

uint32_t Settings::readUInt(Preferences &preferences, bool storageAvailable, const char *key, uint32_t defaultValue)
{
    if (!storageAvailable || !preferences.isKey(key))
    {
        ESP_LOGE("Settings", "Missing setting %s. Using hardcoded default", key);
        return defaultValue;
    }

    return preferences.getUInt(key, defaultValue);
}

uint8_t Settings::readUChar(Preferences &preferences, bool storageAvailable, const char *key, uint8_t defaultValue)
{
    if (!storageAvailable || !preferences.isKey(key))
    {
        ESP_LOGE("Settings", "Missing setting %s. Using hardcoded default", key);
        return defaultValue;
    }

    return preferences.getUChar(key, defaultValue);
}

uint8_t Settings::readCount(Preferences &preferences, bool storageAvailable, const char *key)
{
    static constexpr uint8_t MAX_SETTING_COUNT = 32;

    if (!storageAvailable || !preferences.isKey(key))
    {
        ESP_LOGE("Settings", "Missing setting %s. Using hardcoded default", key);
        return 0;
    }

    uint8_t value = preferences.getUChar(key);
    if (value > MAX_SETTING_COUNT)
    {
        ESP_LOGE("Settings", "Invalid setting %s. Using hardcoded default", key);
        return 0;
    }

    return value;
}

float Settings::readFloat(Preferences &preferences, bool storageAvailable, const char *key, float defaultValue)
{
    if (!storageAvailable || !preferences.isKey(key))
    {
        ESP_LOGE("Settings", "Missing setting %s. Using hardcoded default", key);
        return defaultValue;
    }

    return preferences.getFloat(key, defaultValue);
}

String Settings::readString(Preferences &preferences, bool storageAvailable, const char *key, const String &defaultValue)
{
    if (!storageAvailable || !preferences.isKey(key))
    {
        ESP_LOGE("Settings", "Missing setting %s. Using hardcoded default", key);
        return defaultValue;
    }

    return preferences.getString(key, defaultValue);
}

bool Settings::hasRequiredKey(Preferences &preferences, bool storageAvailable, const char *key)
{
    if (!storageAvailable || !preferences.isKey(key))
    {
        ESP_LOGE("Settings", "Missing required setting %s. Water hub will not start", key);
        return false;
    }

    return true;
}

GlobalSettings Settings::getGlobalSettings()
{
    return globalSettings_;
}

WifiSettings Settings::getWifiSettings()
{
    return wifiSettings_;
}

std::vector<ValveSetting> Settings::getValveSetting()
{
    return valveSettings_;
}

PressureSensorSetting Settings::getPressureSensorSetting()
{
    return pressureSensorSetting_.value();
}

WaterCounterSetting Settings::getMagistralWaterCounterSetting()
{
    return magistralWaterCounterSetting_.value();
}

std::vector<WaterCounterSetting> Settings::getLeafWaterCounterSetting()
{
    return leafWaterCounterSettings_;
}

std::vector<SoilSensorSetting> Settings::getSoilSensorSetting()
{
    return soilSensorSettings_;
}
