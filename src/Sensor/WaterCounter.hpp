#pragma once

#include <Arduino.h>
#include "driver/pulse_cnt.h"
#include <stdint.h>

class WaterCounter
{
public:
    WaterCounter(uint8_t pin, float litersPerTick, uint32_t readIntervalSeconds);
    void readLitersIfDue(uint32_t currentTimeMs);
    void readLiters();
    void setReadIntervalSeconds(uint32_t readIntervalSeconds);
    uint32_t getReadIntervalSeconds() const;
    float getTotalLiters() const;

private:
    pcnt_unit_handle_t pcntUnit_ = NULL;
    float litersPerTick_;
    uint32_t readIntervalSeconds_;
    uint32_t lastReadTimeMs_ = 0;
    uint32_t lastTickCount_ = 0;
    float totalLiters_ = 0;
};
