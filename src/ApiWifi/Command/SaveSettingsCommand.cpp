#include "SaveSettingsCommand.hpp"

#include "Api/JsonRequestReader.hpp"

#include <unordered_set>

SaveSettingsCommand::SaveSettingsCommand(Settings &settings)
    : settings_(settings)
{
}

ApiCommandResult SaveSettingsCommand::execute(const JsonDocument &request)
{
    ApiCommandResult result(400, false);
    SettingsSnapshot snapshot = settings_.get();
    if (!parseSnapshot(request.as<JsonVariantConst>(), snapshot, result.error))
    {
        return result;
    }
    if (!settings_.save(snapshot, result.error))
    {
        result.statusCode = 500;
        return result;
    }

    result.statusCode = 200;
    result.success = true;
    result.data["restartScheduled"] = true;
    return result;
}

bool SaveSettingsCommand::parseSnapshot(JsonVariantConst json, SettingsSnapshot &snapshot, String &error)
{
    static constexpr uint32_t MAX_INTERVAL_SECONDS = INT32_MAX / 1000;
    static constexpr size_t MAX_SETTING_COUNT = 32;

    JsonObjectConst global;
    JsonObjectConst remoteLogSettings;
    JsonArrayConst valves;
    JsonArrayConst leafCounters;
    JsonArrayConst soilSensors;
    if (!JsonRequestReader::readRequiredObject(json, "globalSettings", global, error) ||
        !JsonRequestReader::readRequiredObject(json, "remoteLogSettings", remoteLogSettings, error) ||
        !JsonRequestReader::readRequiredArray(json, "valveSettings", valves, error) ||
        !JsonRequestReader::readRequiredArray(json, "leafWaterCounterSettings", leafCounters, error) ||
        !JsonRequestReader::readRequiredArray(json, "soilSensorSettings", soilSensors, error))
    {
        return false;
    }
    if (valves.size() > MAX_SETTING_COUNT ||
        leafCounters.size() > MAX_SETTING_COUNT ||
        soilSensors.size() > MAX_SETTING_COUNT)
    {
        error = "Settings arrays cannot contain more than 32 items";
        return false;
    }

    GlobalSettings parsedGlobal;
    if (!JsonRequestReader::readRequiredUint32(global, "idleWaterCounterReadIntervalSeconds", parsedGlobal.idleWaterCounterReadIntervalSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "wateringWaterCounterReadIntervalSeconds", parsedGlobal.wateringWaterCounterReadIntervalSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "idlePressureSensorReadIntervalSeconds", parsedGlobal.idlePressureSensorReadIntervalSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "wateringPressureSensorReadIntervalSeconds", parsedGlobal.wateringPressureSensorReadIntervalSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "idleSoilSensorReadIntervalSeconds", parsedGlobal.idleSoilSensorReadIntervalSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "wateringSoilSensorReadIntervalSeconds", parsedGlobal.wateringSoilSensorReadIntervalSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "maximumManualValveOpenTimeSeconds", parsedGlobal.maximumManualValveOpenTimeSeconds, error) ||
        !JsonRequestReader::readRequiredUint8(global, "startWateringBelowHumidityPercent", parsedGlobal.startWateringBelowHumidityPercent, error) ||
        !JsonRequestReader::readRequiredUint8(global, "stopWateringAboveHumidityPercent", parsedGlobal.stopWateringAboveHumidityPercent, error) ||
        !JsonRequestReader::readRequiredUint32(global, "zoneWateringDurationSeconds", parsedGlobal.zoneWateringDurationSeconds, error) ||
        !JsonRequestReader::readRequiredUint32(global, "zoneWateringRetryDelaySeconds", parsedGlobal.zoneWateringRetryDelaySeconds, error))
    {
        return false;
    }
    uint32_t intervals[] = {
        parsedGlobal.idleWaterCounterReadIntervalSeconds,
        parsedGlobal.wateringWaterCounterReadIntervalSeconds,
        parsedGlobal.idlePressureSensorReadIntervalSeconds,
        parsedGlobal.wateringPressureSensorReadIntervalSeconds,
        parsedGlobal.idleSoilSensorReadIntervalSeconds,
        parsedGlobal.wateringSoilSensorReadIntervalSeconds,
        parsedGlobal.maximumManualValveOpenTimeSeconds,
        parsedGlobal.zoneWateringDurationSeconds,
        parsedGlobal.zoneWateringRetryDelaySeconds};
    for (uint32_t value : intervals)
    {
        if (value == 0 || value > MAX_INTERVAL_SECONDS)
        {
            error = "Time intervals must be between 1 and 2147483 seconds";
            return false;
        }
    }
    if (parsedGlobal.startWateringBelowHumidityPercent > 100 ||
        parsedGlobal.stopWateringAboveHumidityPercent > 100 ||
        parsedGlobal.startWateringBelowHumidityPercent >= parsedGlobal.stopWateringAboveHumidityPercent)
    {
        error = "Humidity thresholds must be between 0 and 100 and start must be less than stop";
        return false;
    }

    std::string mode;
    if (!JsonRequestReader::readRequiredString(global, "wateringStartMode", mode, error))
    {
        return false;
    }
    TimeOfDay startTime{0, 0};
    TimeOfDay endTime{0, 0};
    if (mode == WateringStartMode::WITHIN_WATERING_WINDOW)
    {
        JsonObjectConst start;
        JsonObjectConst end;
        if (!JsonRequestReader::readRequiredObject(global, "wateringWindowStartTime", start, error) ||
            !JsonRequestReader::readRequiredObject(global, "wateringWindowEndTime", end, error) ||
            !JsonRequestReader::readRequiredUint8(start, "hour", startTime.hour, error) ||
            !JsonRequestReader::readRequiredUint8(start, "minute", startTime.minute, error) ||
            !JsonRequestReader::readRequiredUint8(end, "hour", endTime.hour, error) ||
            !JsonRequestReader::readRequiredUint8(end, "minute", endTime.minute, error))
        {
            return false;
        }
        if (startTime.hour > 23 || startTime.minute > 59 ||
            endTime.hour > 23 || endTime.minute > 59)
        {
            error = "Watering window start and end times must be valid times of day";
            return false;
        }
    }
    std::optional<WateringStartMode> parsedMode = WateringStartMode::tryFrom(mode.c_str(), startTime, endTime);
    if (!parsedMode.has_value())
    {
        error = String("wateringStartMode must be ") +
                WateringStartMode::IMMEDIATELY +
                " or " +
                WateringStartMode::WITHIN_WATERING_WINDOW;
        return false;
    }
    parsedGlobal.wateringStartMode = parsedMode.value();

    std::string remoteLogUrl;
    std::string remoteLogToken;
    if (!JsonRequestReader::readRequiredString(remoteLogSettings, "url", remoteLogUrl, error) ||
        !JsonRequestReader::readRequiredString(remoteLogSettings, "token", remoteLogToken, error))
    {
        return false;
    }

    uint8_t pressureAddress = 0;
    std::string pressureName = "Pressure sensor";
    std::optional<PressureSensorSetting> parsedPressureSensor;
    JsonVariantConst pressure = json["pressureSensor"];
    if (!pressure.isNull())
    {
        if (!pressure.is<JsonObjectConst>())
        {
            error = "pressureSensor must be an object";
            return false;
        }
        if (!JsonRequestReader::readRequiredUint8(pressure, "slaveAddress", pressureAddress, error))
        {
            return false;
        }
        if (pressureAddress == 0 || pressureAddress > 247)
        {
            error = "pressureSensor.slaveAddress must be between 1 and 247";
            return false;
        }
        JsonVariantConst pressureNameValue = pressure["name"];
        if (!pressureNameValue.isNull())
        {
            if (!pressureNameValue.is<const char *>())
            {
                error = "pressureSensor.name must be a string";
                return false;
            }
            pressureName = pressureNameValue.as<const char *>();
        }
        parsedPressureSensor.emplace(pressureAddress, pressureName);
    }

    uint8_t magistralPin = 0;
    std::string magistralName;
    float magistralLitersPerTick = 0;
    std::optional<WaterCounterSetting> parsedMagistralWaterCounter;
    JsonVariantConst magistral = json["magistralWaterCounterSetting"];
    if (!magistral.isNull())
    {
        if (!magistral.is<JsonObjectConst>())
        {
            error = "magistralWaterCounterSetting must be an object";
            return false;
        }
        if (!JsonRequestReader::readRequiredUint8(magistral, "pin", magistralPin, error) ||
            !JsonRequestReader::readRequiredString(magistral, "name", magistralName, error) ||
            !JsonRequestReader::readRequiredPositiveFloat(magistral, "litersPerTick", magistralLitersPerTick, error))
        {
            return false;
        }
        parsedMagistralWaterCounter.emplace(magistralPin, magistralName, magistralLitersPerTick);
    }

    std::vector<SoilSensorSetting> parsedSoilSensors;
    std::unordered_set<uint8_t> soilAddresses;
    for (JsonObjectConst item : soilSensors)
    {
        uint8_t address = 0;
        std::string name;
        if (!JsonRequestReader::readRequiredUint8(item, "slaveAddress", address, error) ||
            !JsonRequestReader::readRequiredString(item, "name", name, error))
        {
            return false;
        }
        if (address == 0 || address > 247)
        {
            error = "Soil sensor slave address must be between 1 and 247";
            return false;
        }
        if (parsedPressureSensor.has_value() && address == pressureAddress)
        {
            error = "Soil sensor slave address must be different from the pressure sensor address";
            return false;
        }
        if (!soilAddresses.insert(address).second)
        {
            error = "Soil sensor slave addresses must be unique";
            return false;
        }
        parsedSoilSensors.emplace_back(address, name);
    }

    std::unordered_set<uint8_t> usedPins;
    if (parsedMagistralWaterCounter.has_value())
    {
        usedPins.insert(magistralPin);
    }
    std::vector<ValveSetting> parsedValves;
    for (JsonObjectConst item : valves)
    {
        uint8_t pin = 0;
        uint8_t soilAddress = 0;
        std::string name;
        if (!JsonRequestReader::readRequiredUint8(item, "pin", pin, error) ||
            !JsonRequestReader::readRequiredString(item, "name", name, error) ||
            !JsonRequestReader::readRequiredUint8(item, "soilSensorSlaveAddress", soilAddress, error))
        {
            return false;
        }
        if (soilAddresses.count(soilAddress) == 0)
        {
            error = "Each valve must reference an existing soil sensor";
            return false;
        }
        if (!usedPins.insert(pin).second)
        {
            error = "GPIO pins must be unique across valves and water counters";
            return false;
        }
        parsedValves.emplace_back(pin, name, soilAddress);
    }

    std::vector<WaterCounterSetting> parsedLeafCounters;
    for (JsonObjectConst item : leafCounters)
    {
        uint8_t pin = 0;
        std::string name;
        float litersPerTick = 0;
        if (!JsonRequestReader::readRequiredUint8(item, "pin", pin, error) ||
            !JsonRequestReader::readRequiredString(item, "name", name, error) ||
            !JsonRequestReader::readRequiredPositiveFloat(item, "litersPerTick", litersPerTick, error))
        {
            return false;
        }
        if (!usedPins.insert(pin).second)
        {
            error = "GPIO pins must be unique across valves and water counters";
            return false;
        }
        parsedLeafCounters.emplace_back(pin, name, litersPerTick);
    }

    snapshot.globalSettings = parsedGlobal;
    snapshot.remoteLogUrl = remoteLogUrl;
    snapshot.remoteLogToken = remoteLogToken;
    snapshot.valveSettings = parsedValves;
    snapshot.pressureSensorSetting = parsedPressureSensor;
    snapshot.magistralWaterCounterSetting = parsedMagistralWaterCounter;
    snapshot.leafWaterCounterSettings = parsedLeafCounters;
    snapshot.soilSensorSettings = parsedSoilSensors;
    return true;
}
