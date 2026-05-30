#include <Arduino.h>
#include "driver/pulse_cnt.h"
#include <stdint.h>

class WaterCounter
{
public:
    WaterCounter(uint8_t pin, float litersPerTick);
    float getLitersFromLastCall();

private:
    pcnt_unit_handle_t pcntUnit_ = NULL;
    float litersPerTick_;
    uint32_t lastTickCount_ = 0;
};
