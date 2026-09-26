#include "ModbusBusLock.hpp"

#include <Arduino.h>

ModbusBusLock::ModbusBusLock()
    : mutex_(getMutex())
{
    xSemaphoreTake(mutex_, portMAX_DELAY);

    uint32_t elapsedMs = millis() - getLastReleaseTimeMs();
    if (elapsedMs < MinSilentIntervalMs)
    {
        delay(MinSilentIntervalMs - elapsedMs);
    }
}

ModbusBusLock::~ModbusBusLock()
{
    getLastReleaseTimeMs() = millis();
    xSemaphoreGive(mutex_);
}

SemaphoreHandle_t ModbusBusLock::getMutex()
{
    static SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    return mutex;
}

uint32_t &ModbusBusLock::getLastReleaseTimeMs()
{
    static uint32_t lastReleaseTimeMs = 0;
    return lastReleaseTimeMs;
}
