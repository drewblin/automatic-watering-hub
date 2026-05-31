#pragma once

#include <Arduino.h>
#include "driver/pulse_cnt.h"
#include <stdint.h>

class WaterCounter
{
public:
    WaterCounter(uint8_t pin, float litersPerTick);
    void readLiters();
    float getLastReadLiters() const;
    uint32_t getReadingRevision() const;

private:
    pcnt_unit_handle_t pcntUnit_ = NULL;
    float litersPerTick_;
    uint32_t lastTickCount_ = 0;
    float lastReadLiters_ = 0;
    uint32_t readingRevision_ = 0;
};
