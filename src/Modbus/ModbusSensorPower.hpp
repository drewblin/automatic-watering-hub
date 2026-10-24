#pragma once

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class ModbusSensorPower
{
public:
    explicit ModbusSensorPower(uint8_t pin);
    ~ModbusSensorPower();

    ModbusSensorPower(const ModbusSensorPower &) = delete;
    ModbusSensorPower &operator=(const ModbusSensorPower &) = delete;

    void begin();
    void configureWarmup(uint32_t warmupMs);
    void setWateringActive(bool active);
    void setScheduledReadActive(bool active);
    bool isReady();
    void acquire();
    void release();
    void waitUntilReady();

private:
    void updatePowerLocked();

    uint8_t pin_;
    SemaphoreHandle_t mutex_;
    uint32_t warmupMs_ = 0;
    uint32_t poweredSinceMs_ = 0;
    uint32_t activeTransactions_ = 0;
    bool wateringActive_ = false;
    bool scheduledReadActive_ = false;
    bool powered_ = false;
};
