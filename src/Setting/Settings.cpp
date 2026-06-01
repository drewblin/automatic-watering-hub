#include "Settings.hpp"

#include <esp_system.h>

namespace
{
constexpr char STORAGE_NAMESPACE[] = "watering";

constexpr char IDLE_WATER_COUNTER_READ_INTERVAL_KEY[] = "iWcRead";
constexpr char WATERING_WATER_COUNTER_READ_INTERVAL_KEY[] = "wWcRead";
constexpr char IDLE_PRESSURE_SENSOR_READ_INTERVAL_KEY[] = "iPrRead";
constexpr char WATERING_PRESSURE_SENSOR_READ_INTERVAL_KEY[] = "wPrRead";
constexpr char IDLE_SOIL_SENSOR_READ_INTERVAL_KEY[] = "iSoilRead";
constexpr char WATERING_SOIL_SENSOR_READ_INTERVAL_KEY[] = "wSoilRead";
constexpr char MAXIMUM_MANUAL_VALVE_OPEN_TIME_KEY[] = "maxManual";
constexpr char START_WATERING_BELOW_HUMIDITY_KEY[] = "humidStart";
constexpr char STOP_WATERING_ABOVE_HUMIDITY_KEY[] = "humidStop";
constexpr char WATERING_START_MODE_KEY[] = "waterMode";
constexpr char SCHEDULED_WATERING_START_HOUR_KEY[] = "startHour";
constexpr char SCHEDULED_WATERING_START_MINUTE_KEY[] = "startMinute";
constexpr char ZONE_WATERING_DURATION_KEY[] = "zoneDuration";
constexpr char ZONE_WATERING_RETRY_DELAY_KEY[] = "zoneRetry";
constexpr char WIFI_SSID_KEY[] = "wifiSsid";
constexpr char WIFI_PASSWORD_KEY[] = "wifiPassword";
constexpr char API_ACCESS_TOKEN_KEY[] = "apiToken";
constexpr char VALVE_COUNT_KEY[] = "valveCount";
constexpr char PRESSURE_SENSOR_ADDRESS_KEY[] = "pressureAddr";
constexpr char MAGISTRAL_WATER_COUNTER_PIN_KEY[] = "mainWcPin";
constexpr char MAGISTRAL_WATER_COUNTER_NAME_KEY[] = "mainWcName";
constexpr char MAGISTRAL_WATER_COUNTER_LITERS_PER_TICK_KEY[] = "mainWcLpt";
constexpr char LEAF_WATER_COUNTER_COUNT_KEY[] = "leafWcCount";
constexpr char SOIL_SENSOR_COUNT_KEY[] = "soilCount";

constexpr char VALVE_PREFIX[] = "v";
constexpr char LEAF_WATER_COUNTER_PREFIX[] = "l";
constexpr char SOIL_SENSOR_PREFIX[] = "s";
constexpr char PIN_SUFFIX[] = "Pin";
constexpr char NAME_SUFFIX[] = "Name";
constexpr char SOIL_SENSOR_SUFFIX[] = "Soil";
constexpr char LITERS_PER_TICK_SUFFIX[] = "Lpt";
constexpr char ADDRESS_SUFFIX[] = "Addr";
}

Settings::Settings()
{
}

void Settings::begin()
{
    Preferences preferences;
    bool storageAvailable = preferences.begin(STORAGE_NAMESPACE, true);
    if (!storageAvailable)
    {
        ESP_LOGE("Settings", "Failed to open NVS namespace. Using hardcoded defaults where available");
    }

    snapshot_.globalSettings.idleWaterCounterReadIntervalSeconds = readUInt(preferences, storageAvailable, IDLE_WATER_COUNTER_READ_INTERVAL_KEY, 5 * 60);
    snapshot_.globalSettings.wateringWaterCounterReadIntervalSeconds = readUInt(preferences, storageAvailable, WATERING_WATER_COUNTER_READ_INTERVAL_KEY, 1 * 60);
    snapshot_.globalSettings.idlePressureSensorReadIntervalSeconds = readUInt(preferences, storageAvailable, IDLE_PRESSURE_SENSOR_READ_INTERVAL_KEY, 5 * 60);
    snapshot_.globalSettings.wateringPressureSensorReadIntervalSeconds = readUInt(preferences, storageAvailable, WATERING_PRESSURE_SENSOR_READ_INTERVAL_KEY, 5);
    snapshot_.globalSettings.idleSoilSensorReadIntervalSeconds = readUInt(preferences, storageAvailable, IDLE_SOIL_SENSOR_READ_INTERVAL_KEY, 60 * 60);
    snapshot_.globalSettings.wateringSoilSensorReadIntervalSeconds = readUInt(preferences, storageAvailable, WATERING_SOIL_SENSOR_READ_INTERVAL_KEY, 1 * 60);
    snapshot_.globalSettings.maximumManualValveOpenTimeSeconds = readUInt(preferences, storageAvailable, MAXIMUM_MANUAL_VALVE_OPEN_TIME_KEY, 3600);
    snapshot_.globalSettings.startWateringBelowHumidityPercent = readUChar(preferences, storageAvailable, START_WATERING_BELOW_HUMIDITY_KEY, 20);
    snapshot_.globalSettings.stopWateringAboveHumidityPercent = readUChar(preferences, storageAvailable, STOP_WATERING_ABOVE_HUMIDITY_KEY, 80);

    TimeOfDay scheduledWateringStartTime{
        readUChar(preferences, storageAvailable, SCHEDULED_WATERING_START_HOUR_KEY, 23),
        readUChar(preferences, storageAvailable, SCHEDULED_WATERING_START_MINUTE_KEY, 10)};
    String wateringStartModeValue = readString(
        preferences,
        storageAvailable,
        WATERING_START_MODE_KEY,
        WateringStartMode::IMMEDIATELY);
    std::optional<WateringStartMode> wateringStartMode = WateringStartMode::tryFrom(
        wateringStartModeValue.c_str(),
        scheduledWateringStartTime);
    if (!wateringStartMode.has_value())
    {
        ESP_LOGE("Settings", "Invalid setting %s. Using hardcoded default", WATERING_START_MODE_KEY);
        wateringStartMode = WateringStartMode::immediately();
    }
    snapshot_.globalSettings.wateringStartMode = wateringStartMode.value();

    snapshot_.globalSettings.zoneWateringDurationSeconds = readUInt(preferences, storageAvailable, ZONE_WATERING_DURATION_KEY, 5 * 60);
    snapshot_.globalSettings.zoneWateringRetryDelaySeconds = readUInt(preferences, storageAvailable, ZONE_WATERING_RETRY_DELAY_KEY, 15 * 60);

    snapshot_.wifiSettings.ssid = readString(preferences, storageAvailable, WIFI_SSID_KEY, "").c_str();
    snapshot_.wifiSettings.password = readString(preferences, storageAvailable, WIFI_PASSWORD_KEY, "").c_str();
    snapshot_.apiAccessToken = readString(preferences, storageAvailable, API_ACCESS_TOKEN_KEY, "").c_str();

    uint8_t valveCount = readCount(preferences, storageAvailable, VALVE_COUNT_KEY);
    snapshot_.valveSettings.clear();
    snapshot_.valveSettings.reserve(valveCount);
    for (uint8_t i = 0; i < valveCount; ++i)
    {
        String prefix = VALVE_PREFIX + String(i);
        snapshot_.valveSettings.emplace_back(
            readUChar(preferences, storageAvailable, (prefix + PIN_SUFFIX).c_str(), 19),
            readString(preferences, storageAvailable, (prefix + NAME_SUFFIX).c_str(), "").c_str(),
            readUChar(preferences, storageAvailable, (prefix + SOIL_SENSOR_SUFFIX).c_str(), 1));
    }

    bool hasPressureSensorSetting = hasRequiredKey(preferences, storageAvailable, PRESSURE_SENSOR_ADDRESS_KEY);
    snapshot_.pressureSensorSetting.reset();
    if (hasPressureSensorSetting)
    {
        snapshot_.pressureSensorSetting.emplace(preferences.getUChar(PRESSURE_SENSOR_ADDRESS_KEY));
    }

    bool hasMagistralWaterCounterPin = hasRequiredKey(preferences, storageAvailable, MAGISTRAL_WATER_COUNTER_PIN_KEY);
    bool hasMagistralWaterCounterName = hasRequiredKey(preferences, storageAvailable, MAGISTRAL_WATER_COUNTER_NAME_KEY);
    bool hasMagistralWaterCounterLitersPerTick = hasRequiredKey(preferences, storageAvailable, MAGISTRAL_WATER_COUNTER_LITERS_PER_TICK_KEY);
    snapshot_.magistralWaterCounterSetting.reset();
    if (hasMagistralWaterCounterPin &&
        hasMagistralWaterCounterName &&
        hasMagistralWaterCounterLitersPerTick)
    {
        snapshot_.magistralWaterCounterSetting.emplace(
            preferences.getUChar(MAGISTRAL_WATER_COUNTER_PIN_KEY),
            preferences.getString(MAGISTRAL_WATER_COUNTER_NAME_KEY).c_str(),
            preferences.getFloat(MAGISTRAL_WATER_COUNTER_LITERS_PER_TICK_KEY));
    }

    uint8_t leafWaterCounterCount = readCount(preferences, storageAvailable, LEAF_WATER_COUNTER_COUNT_KEY);
    snapshot_.leafWaterCounterSettings.clear();
    snapshot_.leafWaterCounterSettings.reserve(leafWaterCounterCount);
    for (uint8_t i = 0; i < leafWaterCounterCount; ++i)
    {
        String prefix = LEAF_WATER_COUNTER_PREFIX + String(i);
        snapshot_.leafWaterCounterSettings.emplace_back(
            readUChar(preferences, storageAvailable, (prefix + PIN_SUFFIX).c_str(), 36),
            readString(preferences, storageAvailable, (prefix + NAME_SUFFIX).c_str(), "").c_str(),
            readFloat(preferences, storageAvailable, (prefix + LITERS_PER_TICK_SUFFIX).c_str(), 1));
    }

    uint8_t soilSensorCount = readCount(preferences, storageAvailable, SOIL_SENSOR_COUNT_KEY);
    snapshot_.soilSensorSettings.clear();
    snapshot_.soilSensorSettings.reserve(soilSensorCount);
    for (uint8_t i = 0; i < soilSensorCount; ++i)
    {
        String prefix = SOIL_SENSOR_PREFIX + String(i);
        snapshot_.soilSensorSettings.emplace_back(
            readUChar(preferences, storageAvailable, (prefix + ADDRESS_SUFFIX).c_str(), 1),
            readString(preferences, storageAvailable, (prefix + NAME_SUFFIX).c_str(), "").c_str());
    }

    if (storageAvailable)
    {
        preferences.end();
    }

    if (snapshot_.apiAccessToken.empty())
    {
        snapshot_.apiAccessToken = generateApiAccessToken();
        Preferences writablePreferences;
        bool writableStorageAvailable = writablePreferences.begin(STORAGE_NAMESPACE, false);
        if (!writableStorageAvailable ||
            !putString(writablePreferences, API_ACCESS_TOKEN_KEY, snapshot_.apiAccessToken.c_str()))
        {
            ESP_LOGE("Settings", "Failed to persist generated API access token");
        }
        if (writableStorageAvailable)
        {
            writablePreferences.end();
        }
    }
}

bool Settings::save(const SettingsSnapshot &snapshot, String &error)
{
    Preferences preferences;
    if (!preferences.begin(STORAGE_NAMESPACE, false))
    {
        error = "Failed to open settings storage";
        return false;
    }

    std::optional<TimeOfDay> scheduledStartTime = snapshot.globalSettings.wateringStartMode.getScheduledStartTime();
    TimeOfDay storedStartTime = scheduledStartTime.value_or(TimeOfDay{0, 0});
    bool saved =
        preferences.putUInt(IDLE_WATER_COUNTER_READ_INTERVAL_KEY, snapshot.globalSettings.idleWaterCounterReadIntervalSeconds) > 0 &&
        preferences.putUInt(WATERING_WATER_COUNTER_READ_INTERVAL_KEY, snapshot.globalSettings.wateringWaterCounterReadIntervalSeconds) > 0 &&
        preferences.putUInt(IDLE_PRESSURE_SENSOR_READ_INTERVAL_KEY, snapshot.globalSettings.idlePressureSensorReadIntervalSeconds) > 0 &&
        preferences.putUInt(WATERING_PRESSURE_SENSOR_READ_INTERVAL_KEY, snapshot.globalSettings.wateringPressureSensorReadIntervalSeconds) > 0 &&
        preferences.putUInt(IDLE_SOIL_SENSOR_READ_INTERVAL_KEY, snapshot.globalSettings.idleSoilSensorReadIntervalSeconds) > 0 &&
        preferences.putUInt(WATERING_SOIL_SENSOR_READ_INTERVAL_KEY, snapshot.globalSettings.wateringSoilSensorReadIntervalSeconds) > 0 &&
        preferences.putUInt(MAXIMUM_MANUAL_VALVE_OPEN_TIME_KEY, snapshot.globalSettings.maximumManualValveOpenTimeSeconds) > 0 &&
        preferences.putUChar(START_WATERING_BELOW_HUMIDITY_KEY, snapshot.globalSettings.startWateringBelowHumidityPercent) > 0 &&
        preferences.putUChar(STOP_WATERING_ABOVE_HUMIDITY_KEY, snapshot.globalSettings.stopWateringAboveHumidityPercent) > 0 &&
        preferences.putString(WATERING_START_MODE_KEY, snapshot.globalSettings.wateringStartMode.toString()) > 0 &&
        preferences.putUChar(SCHEDULED_WATERING_START_HOUR_KEY, storedStartTime.hour) > 0 &&
        preferences.putUChar(SCHEDULED_WATERING_START_MINUTE_KEY, storedStartTime.minute) > 0 &&
        preferences.putUInt(ZONE_WATERING_DURATION_KEY, snapshot.globalSettings.zoneWateringDurationSeconds) > 0 &&
        preferences.putUInt(ZONE_WATERING_RETRY_DELAY_KEY, snapshot.globalSettings.zoneWateringRetryDelaySeconds) > 0 &&
        putString(preferences, WIFI_SSID_KEY, snapshot.wifiSettings.ssid.c_str()) &&
        putString(preferences, WIFI_PASSWORD_KEY, snapshot.wifiSettings.password.c_str());

    if (saved && snapshot.hasPressureSensorSetting())
    {
        saved = preferences.putUChar(PRESSURE_SENSOR_ADDRESS_KEY, snapshot.pressureSensorSetting->getSlaveAddress()) > 0;
    }
    else if (saved && preferences.isKey(PRESSURE_SENSOR_ADDRESS_KEY))
    {
        saved = preferences.remove(PRESSURE_SENSOR_ADDRESS_KEY);
    }

    if (saved && snapshot.hasMagistralWaterCounterSetting())
    {
        saved =
            preferences.putUChar(MAGISTRAL_WATER_COUNTER_PIN_KEY, snapshot.magistralWaterCounterSetting->getPin()) > 0 &&
            putString(preferences, MAGISTRAL_WATER_COUNTER_NAME_KEY, snapshot.magistralWaterCounterSetting->getName().c_str()) &&
            preferences.putFloat(MAGISTRAL_WATER_COUNTER_LITERS_PER_TICK_KEY, snapshot.magistralWaterCounterSetting->getLitersPerTick()) > 0;
    }
    else if (saved)
    {
        const char *magistralWaterCounterKeys[] = {
            MAGISTRAL_WATER_COUNTER_PIN_KEY,
            MAGISTRAL_WATER_COUNTER_NAME_KEY,
            MAGISTRAL_WATER_COUNTER_LITERS_PER_TICK_KEY};
        for (const char *key : magistralWaterCounterKeys)
        {
            if (preferences.isKey(key) && !preferences.remove(key))
            {
                saved = false;
                break;
            }
        }
    }

    for (uint8_t i = 0; saved && i < snapshot.valveSettings.size(); ++i)
    {
        String prefix = VALVE_PREFIX + String(i);
        saved =
            preferences.putUChar((prefix + PIN_SUFFIX).c_str(), snapshot.valveSettings[i].getPin()) > 0 &&
            putString(preferences, (prefix + NAME_SUFFIX).c_str(), snapshot.valveSettings[i].getName().c_str()) &&
            preferences.putUChar((prefix + SOIL_SENSOR_SUFFIX).c_str(), snapshot.valveSettings[i].getSoilSensorSlaveAddress()) > 0;
    }
    for (uint8_t i = 0; saved && i < snapshot.leafWaterCounterSettings.size(); ++i)
    {
        String prefix = LEAF_WATER_COUNTER_PREFIX + String(i);
        saved =
            preferences.putUChar((prefix + PIN_SUFFIX).c_str(), snapshot.leafWaterCounterSettings[i].getPin()) > 0 &&
            putString(preferences, (prefix + NAME_SUFFIX).c_str(), snapshot.leafWaterCounterSettings[i].getName().c_str()) &&
            preferences.putFloat((prefix + LITERS_PER_TICK_SUFFIX).c_str(), snapshot.leafWaterCounterSettings[i].getLitersPerTick()) > 0;
    }
    for (uint8_t i = 0; saved && i < snapshot.soilSensorSettings.size(); ++i)
    {
        String prefix = SOIL_SENSOR_PREFIX + String(i);
        saved =
            preferences.putUChar((prefix + ADDRESS_SUFFIX).c_str(), snapshot.soilSensorSettings[i].getSlaveAddress()) > 0 &&
            putString(preferences, (prefix + NAME_SUFFIX).c_str(), snapshot.soilSensorSettings[i].getName().c_str());
    }
    saved =
        saved &&
        preferences.putUChar(VALVE_COUNT_KEY, snapshot.valveSettings.size()) > 0 &&
        preferences.putUChar(LEAF_WATER_COUNTER_COUNT_KEY, snapshot.leafWaterCounterSettings.size()) > 0 &&
        preferences.putUChar(SOIL_SENSOR_COUNT_KEY, snapshot.soilSensorSettings.size()) > 0;
    preferences.end();

    if (!saved)
    {
        error = "Failed to save settings";
        return false;
    }

    return true;
}

bool Settings::saveWifiSettings(const WifiSettings &wifiSettings, String &error)
{
    Preferences preferences;
    if (!preferences.begin(STORAGE_NAMESPACE, false))
    {
        error = "Failed to open settings storage";
        return false;
    }

    bool saved =
        putString(preferences, WIFI_SSID_KEY, wifiSettings.ssid.c_str()) &&
        putString(preferences, WIFI_PASSWORD_KEY, wifiSettings.password.c_str());
    preferences.end();

    if (!saved)
    {
        error = "Failed to save WiFi settings";
        return false;
    }

    return true;
}

std::string Settings::generateApiAccessToken()
{
    static constexpr char HEX_DIGITS[] = "0123456789abcdef";
    uint8_t randomBytes[32];
    esp_fill_random(randomBytes, sizeof(randomBytes));

    std::string token;
    token.reserve(sizeof(randomBytes) * 2);
    for (uint8_t value : randomBytes)
    {
        token += HEX_DIGITS[value >> 4];
        token += HEX_DIGITS[value & 0x0f];
    }
    return token;
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

bool Settings::putString(Preferences &preferences, const char *key, const char *value)
{
    size_t savedLength = preferences.putString(key, value);
    return savedLength == strlen(value) && preferences.getString(key) == value;
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

SettingsSnapshot Settings::get() const
{
    return snapshot_;
}
