#pragma once

#include <stdint.h>

struct GlobalSettings
{
    uint32_t waterCounterReadIntervalSeconds;
    uint32_t pressureSensorReadIntervalSeconds;
    uint32_t soilSensorReadIntervalSeconds;
};
