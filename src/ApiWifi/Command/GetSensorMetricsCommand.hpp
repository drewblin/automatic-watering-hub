#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Hub/WaterHub.hpp"
#include "Setting/SettingsSnapshot.hpp"

class GetSensorMetricsCommand
{
public:
    GetSensorMetricsCommand(WaterHub &waterHub, const SettingsSnapshot &settings);

    ApiCommandResult execute();

private:
    WaterHub &waterHub_;
    const SettingsSnapshot settings_;

    void addSensorMetric(
        JsonArray &metrics,
        uint8_t sensorId,
        const char *sensorType,
        const std::string &name,
        float value,
        uint32_t uptimeMs);
};
