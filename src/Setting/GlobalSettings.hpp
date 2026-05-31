#pragma once

#include <stdint.h>

struct GlobalSettings
{
    uint32_t idleWaterCounterReadIntervalSeconds;
    uint32_t wateringWaterCounterReadIntervalSeconds;
    uint32_t idlePressureSensorReadIntervalSeconds;
    uint32_t wateringPressureSensorReadIntervalSeconds;
    uint32_t idleSoilSensorReadIntervalSeconds;
    uint32_t wateringSoilSensorReadIntervalSeconds;
    uint32_t maximumManualValveOpenTimeSeconds;
};
