#include "Hypervisor.hpp"

#include <Arduino.h>
#include "Logging/Logger.hpp"

Hypervisor::Hypervisor(WaterHub &waterHub) : waterHub_(waterHub)
{
}

void Hypervisor::begin()
{
    closeAllValves();
}

void Hypervisor::loop()
{
    closeExpiredValves();
    checkUnauthorizedWaterFlow();
    checkOpenValvesWithoutWaterFlow();
}

void Hypervisor::checkUnauthorizedWaterFlow()
{
    const WaterCounter *magistralCounter = waterHub_.getMagistralWaterCounter();
    if (!hasNewMagistralWaterCounterReading(
            unauthorizedFlowMagistralWaterCounterReadingRevision_))
    {
        return;
    }

    float leafWaterUsageLiters = 0;
    for (const auto &counter : waterHub_.getLeafWaterCounters())
    {
        leafWaterUsageLiters += counter->getLastReadLiters();
    }

    const float unaccountedWaterUsageLiters =
        magistralCounter->getLastReadLiters() - leafWaterUsageLiters;
    if (unaccountedWaterUsageLiters <= WATER_COUNTER_TOLERANCE_LITERS)
    {
        unauthorizedFlowDetected_ = false;
        unauthorizedFlowErrorLogged_ = false;
        return;
    }

    if (!unauthorizedFlowDetected_)
    {
        for (const auto &valve : waterHub_.getValves())
        {
            if (valve->isOpen())
            {
                return;
            }
        }

        Logger::w(
            "Hypervisor",
            "Unauthorized water flow detected: %.2f liters are not accounted for by leaf counters. Closing all valves",
            unaccountedWaterUsageLiters);

        closeAllValves();
        unauthorizedFlowDetectedTimeMs_ = millis();
        unauthorizedFlowDetected_ = true;
        return;
    }

    if (!unauthorizedFlowErrorLogged_ &&
        millis() - unauthorizedFlowDetectedTimeMs_ >= UNAUTHORIZED_FLOW_ERROR_DELAY_MS)
    {
        Logger::e(
            "Hypervisor",
            "Unauthorized water flow persists after closing all valves: %.2f liters are not accounted for by leaf counters",
            unaccountedWaterUsageLiters);

        unauthorizedFlowErrorLogged_ = true;
    }
}

void Hypervisor::checkOpenValvesWithoutWaterFlow()
{
    if (!hasNewMagistralWaterCounterReading(
            openValveWithoutFlowMagistralWaterCounterReadingRevision_))
    {
        return;
    }

    bool hasOpenValve = false;
    for (const auto &valve : waterHub_.getValves())
    {
        if (valve->isOpen())
        {
            hasOpenValve = true;
            break;
        }
    }

    const bool hasMagistralWaterFlow =
        waterHub_.getMagistralWaterCounter()->getLastReadLiters() > WATER_COUNTER_TOLERANCE_LITERS;
    if (!hasOpenValve || hasMagistralWaterFlow)
    {
        openValveWithoutFlowDetected_ = false;
        openValveWithoutFlowErrorLogged_ = false;
        return;
    }

    if (!openValveWithoutFlowDetected_)
    {
        openValveWithoutFlowDetectedTimeMs_ = millis();
        openValveWithoutFlowDetected_ = true;
        return;
    }

    if (!openValveWithoutFlowErrorLogged_ &&
        millis() - openValveWithoutFlowDetectedTimeMs_ >= OPEN_VALVE_WITHOUT_FLOW_ERROR_DELAY_MS)
    {
        Logger::e(
            "Hypervisor",
            "At least one valve has been open for more than 10 seconds, but the magistral water counter shows no flow");

        openValveWithoutFlowErrorLogged_ = true;
    }
}

bool Hypervisor::hasNewMagistralWaterCounterReading(uint32_t &lastProcessedRevision) const
{
    const uint32_t currentRevision =
        waterHub_.getMagistralWaterCounter()->getReadingRevision();
    if (currentRevision == lastProcessedRevision)
    {
        return false;
    }

    lastProcessedRevision = currentRevision;
    return true;
}

void Hypervisor::closeAllValves()
{
    for (const auto &valve : waterHub_.getValves())
    {
        valve->close();
    }
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
