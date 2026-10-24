#include "ModbusSensorPower.hpp"

#include <Arduino.h>

ModbusSensorPower::ModbusSensorPower(uint8_t pin)
    : pin_(pin), mutex_(xSemaphoreCreateMutex())
{
    configASSERT(mutex_ != nullptr);
}

ModbusSensorPower::~ModbusSensorPower()
{
    vSemaphoreDelete(mutex_);
}

void ModbusSensorPower::begin()
{
    digitalWrite(pin_, LOW);
    pinMode(pin_, OUTPUT);
}

void ModbusSensorPower::configureWarmup(uint32_t warmupMs)
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    warmupMs_ = warmupMs;
    xSemaphoreGive(mutex_);
}

void ModbusSensorPower::setWateringActive(bool active)
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    wateringActive_ = active;
    updatePowerLocked();
    xSemaphoreGive(mutex_);
}

void ModbusSensorPower::setScheduledReadActive(bool active)
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    scheduledReadActive_ = active;
    updatePowerLocked();
    xSemaphoreGive(mutex_);
}

bool ModbusSensorPower::isReady()
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    const bool ready = powered_ && millis() - poweredSinceMs_ >= warmupMs_;
    xSemaphoreGive(mutex_);
    return ready;
}

void ModbusSensorPower::acquire()
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    ++activeTransactions_;
    updatePowerLocked();
    xSemaphoreGive(mutex_);
}

void ModbusSensorPower::release()
{
    xSemaphoreTake(mutex_, portMAX_DELAY);
    configASSERT(activeTransactions_ > 0);
    --activeTransactions_;
    updatePowerLocked();
    xSemaphoreGive(mutex_);
}

void ModbusSensorPower::waitUntilReady()
{
    while (true)
    {
        xSemaphoreTake(mutex_, portMAX_DELAY);
        const uint32_t elapsedMs = millis() - poweredSinceMs_;
        const uint32_t requiredMs = warmupMs_;
        xSemaphoreGive(mutex_);

        if (elapsedMs >= requiredMs)
        {
            return;
        }
        delay(requiredMs - elapsedMs);
    }
}

void ModbusSensorPower::updatePowerLocked()
{
    const bool shouldBePowered = wateringActive_ || scheduledReadActive_ || activeTransactions_ > 0;
    if (shouldBePowered == powered_)
    {
        return;
    }

    digitalWrite(pin_, shouldBePowered ? HIGH : LOW);
    powered_ = shouldBePowered;
    if (powered_)
    {
        poweredSinceMs_ = millis();
    }
}
