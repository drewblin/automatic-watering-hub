#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

class ModbusBusLock
{
public:
    ModbusBusLock();
    ~ModbusBusLock();

    ModbusBusLock(const ModbusBusLock &) = delete;
    ModbusBusLock &operator=(const ModbusBusLock &) = delete;

private:
    static constexpr uint32_t MinSilentIntervalMs = 100;

    static SemaphoreHandle_t getMutex();
    static uint32_t &getLastReleaseTimeMs();

    SemaphoreHandle_t mutex_;
};
