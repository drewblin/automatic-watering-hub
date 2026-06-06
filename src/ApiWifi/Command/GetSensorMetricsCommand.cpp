#include "GetSensorMetricsCommand.hpp"

#include <Arduino.h>
#include <algorithm>

GetSensorMetricsCommand::GetSensorMetricsCommand(WaterHub &waterHub, const SettingsSnapshot &settings)
    : waterHub_(waterHub), settings_(settings)
{
}

ApiCommandResult GetSensorMetricsCommand::execute()
{
    ApiCommandResult result(200, true);
    JsonArray metrics = result.data["sensors"].to<JsonArray>();
    uint32_t uptimeMs = millis();

    const PressureSensor *pressureSensor = waterHub_.getPressureSensor();
    if (pressureSensor != nullptr && settings_.pressureSensorSetting.has_value())
    {
        const PressureSensorSetting &setting = settings_.pressureSensorSetting.value();
        addSensorMetric(
            metrics,
            setting.getSlaveAddress(),
            "pressure",
            setting.getName(),
            pressureSensor->getLastReadPressure(),
            uptimeMs);
    }

    const WaterCounter *magistralWaterCounter = waterHub_.getMagistralWaterCounter();
    if (magistralWaterCounter != nullptr && settings_.magistralWaterCounterSetting.has_value())
    {
        const WaterCounterSetting &setting = settings_.magistralWaterCounterSetting.value();
        addSensorMetric(
            metrics,
            setting.getPin(),
            "water_counter",
            setting.getName(),
            magistralWaterCounter->getTotalLiters(),
            uptimeMs);
    }

    const auto &leafWaterCounters = waterHub_.getLeafWaterCounters();
    size_t leafWaterCounterCount = std::min(leafWaterCounters.size(), settings_.leafWaterCounterSettings.size());
    for (size_t i = 0; i < leafWaterCounterCount; ++i)
    {
        const WaterCounterSetting &setting = settings_.leafWaterCounterSettings[i];
        addSensorMetric(
            metrics,
            setting.getPin(),
            "water_counter",
            setting.getName(),
            leafWaterCounters[i]->getTotalLiters(),
            uptimeMs);
    }

    const auto &soilSensors = waterHub_.getSoilSensors();
    size_t soilSensorCount = std::min(soilSensors.size(), settings_.soilSensorSettings.size());
    for (size_t i = 0; i < soilSensorCount; ++i)
    {
        const SoilSensorSetting &setting = settings_.soilSensorSettings[i];
        addSensorMetric(
            metrics,
            setting.getSlaveAddress(),
            "soil_temperature",
            setting.getName(),
            soilSensors[i]->getLastReadTemperature(),
            uptimeMs);
        addSensorMetric(
            metrics,
            setting.getSlaveAddress(),
            "soil_humidity",
            setting.getName(),
            soilSensors[i]->getLastReadHumidity(),
            uptimeMs);
    }

    return result;
}

void GetSensorMetricsCommand::addSensorMetric(
    JsonArray &metrics,
    uint8_t sensorId,
    const char *sensorType,
    const std::string &name,
    float value,
    uint32_t uptimeMs)
{
    JsonObject sensorMetric = metrics.add<JsonObject>();
    sensorMetric["sensorId"] = sensorId;
    sensorMetric["sensorType"] = sensorType;
    sensorMetric["name"] = name;
    sensorMetric["value"] = value;
    sensorMetric["uptimeMs"] = uptimeMs;
}
