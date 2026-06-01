#include "GetSettingsCommand.hpp"

#include <ctime>

GetSettingsCommand::GetSettingsCommand(const SettingsSnapshot &settings, Clock &clock)
    : settings_(settings), clock_(clock)
{
}

ApiCommandResult GetSettingsCommand::execute()
{
    ApiCommandResult result(200, true);
    JsonObject target = result.data["settings"].to<JsonObject>();
    GlobalSettings globalSettings = settings_.globalSettings;
    JsonObject jsonGlobalSettings = target["globalSettings"].to<JsonObject>();
    jsonGlobalSettings["idleWaterCounterReadIntervalSeconds"] = globalSettings.idleWaterCounterReadIntervalSeconds;
    jsonGlobalSettings["wateringWaterCounterReadIntervalSeconds"] = globalSettings.wateringWaterCounterReadIntervalSeconds;
    jsonGlobalSettings["idlePressureSensorReadIntervalSeconds"] = globalSettings.idlePressureSensorReadIntervalSeconds;
    jsonGlobalSettings["wateringPressureSensorReadIntervalSeconds"] = globalSettings.wateringPressureSensorReadIntervalSeconds;
    jsonGlobalSettings["idleSoilSensorReadIntervalSeconds"] = globalSettings.idleSoilSensorReadIntervalSeconds;
    jsonGlobalSettings["wateringSoilSensorReadIntervalSeconds"] = globalSettings.wateringSoilSensorReadIntervalSeconds;
    jsonGlobalSettings["maximumManualValveOpenTimeSeconds"] = globalSettings.maximumManualValveOpenTimeSeconds;
    jsonGlobalSettings["startWateringBelowHumidityPercent"] = globalSettings.startWateringBelowHumidityPercent;
    jsonGlobalSettings["stopWateringAboveHumidityPercent"] = globalSettings.stopWateringAboveHumidityPercent;
    jsonGlobalSettings["wateringStartMode"] = globalSettings.wateringStartMode.toString();
    std::optional<TimeOfDay> scheduledStartTime = globalSettings.wateringStartMode.getScheduledStartTime();
    if (scheduledStartTime.has_value())
    {
        JsonObject jsonTime = jsonGlobalSettings["scheduledWateringStartTime"].to<JsonObject>();
        jsonTime["hour"] = scheduledStartTime->hour;
        jsonTime["minute"] = scheduledStartTime->minute;
    }
    else
    {
        jsonGlobalSettings["scheduledWateringStartTime"] = nullptr;
    }
    jsonGlobalSettings["zoneWateringDurationSeconds"] = globalSettings.zoneWateringDurationSeconds;
    jsonGlobalSettings["zoneWateringRetryDelaySeconds"] = globalSettings.zoneWateringRetryDelaySeconds;

    WifiSettings wifiSettings = settings_.wifiSettings;
    target["wifiSettings"]["ssid"] = wifiSettings.ssid;
    target["wifiSettings"]["password"] = wifiSettings.password;

    JsonArray valves = target["valveSettings"].to<JsonArray>();
    for (ValveSetting setting : settings_.valveSettings)
    {
        JsonObject json = valves.add<JsonObject>();
        json["pin"] = setting.getPin();
        json["name"] = setting.getName();
        json["soilSensorSlaveAddress"] = setting.getSoilSensorSlaveAddress();
    }

    if (settings_.hasPressureSensorSetting())
    {
        target["pressureSensor"]["slaveAddress"] = settings_.pressureSensorSetting->getSlaveAddress();
    }
    else
    {
        target["pressureSensor"] = nullptr;
    }

    if (settings_.hasMagistralWaterCounterSetting())
    {
        WaterCounterSetting magistral = settings_.magistralWaterCounterSetting.value();
        target["magistralWaterCounterSetting"]["pin"] = magistral.getPin();
        target["magistralWaterCounterSetting"]["name"] = magistral.getName();
        target["magistralWaterCounterSetting"]["litersPerTick"] = magistral.getLitersPerTick();
    }
    else
    {
        target["magistralWaterCounterSetting"] = nullptr;
    }

    JsonArray counters = target["leafWaterCounterSettings"].to<JsonArray>();
    for (WaterCounterSetting setting : settings_.leafWaterCounterSettings)
    {
        JsonObject json = counters.add<JsonObject>();
        json["pin"] = setting.getPin();
        json["name"] = setting.getName();
        json["litersPerTick"] = setting.getLitersPerTick();
    }

    JsonArray soilSensors = target["soilSensorSettings"].to<JsonArray>();
    for (SoilSensorSetting setting : settings_.soilSensorSettings)
    {
        JsonObject json = soilSensors.add<JsonObject>();
        json["slaveAddress"] = setting.getSlaveAddress();
        json["name"] = setting.getName();
    }

    std::optional<std::time_t> currentTime = clock_.now();
    if (currentTime.has_value())
    {
        result.data["controllerCurrentTimestamp"] = currentTime.value();
        std::tm localTime;
        localtime_r(&currentTime.value(), &localTime);
        char formattedTime[32];
        std::strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%dT%H:%M:%S%z", &localTime);
        result.data["controllerCurrentTime"] = formattedTime;
    }
    else
    {
        result.data["controllerCurrentTimestamp"] = nullptr;
        result.data["controllerCurrentTime"] = nullptr;
    }

    return result;
}
