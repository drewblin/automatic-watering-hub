#include "Hypervisor.hpp"

#include <Arduino.h>

Hypervisor::Hypervisor(WaterHub &waterHub) : waterHub_(waterHub)
{
}

void Hypervisor::begin()
{
    for (const auto &valve : waterHub_.getValves())
    {
        valve->close();
    }
}

void Hypervisor::loop()
{
    closeExpiredValves();
}

void Hypervisor::closeExpiredValves()
{
    uint32_t currentTimeMs = millis();

    for (const auto &valve : waterHub_.getValves())
    {
        if (!valve->isOpenExpired(currentTimeMs))
        {
            continue;
        }

        valve->close();
    }
}
