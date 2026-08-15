#include "Hypervisor.hpp"

#include <Arduino.h>
#include "Logging/Logger.hpp"

Hypervisor::Hypervisor(const SettingsSnapshot &settings, WaterHub &waterHub)
    : settings_(settings), waterHub_(waterHub)
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
    if (magistralCounter == nullptr)
    {
        unauthorizedFlowDetected_ = false;
        unauthorizedFlowErrorLogged_ = false;
        return;
    }

    if (waterHub_.hasOpenValve())
    {
        unauthorizedFlowDetected_ = false;
        unauthorizedFlowErrorLogged_ = false;
        unauthorizedFlowWateringWorked_ = true;
        return;
    }

    const uint32_t now = millis();
    if (now - unauthorizedFlowLastCheckTimeMs_ < settings_.globalSettings.idleWaterCounterReadIntervalSeconds * 1000)
    {
        return;
    }

    unauthorizedFlowLastCheckTimeMs_ = now;
    const float currentMagistralLiters = magistralCounter->getTotalLiters();
    float currentLeafLiters = 0;
    for (const auto &counter : waterHub_.getLeafWaterCounters())
    {
        currentLeafLiters += counter->getTotalLiters();
    }

    const float unauthorizedWaterUsageLiters =
        (currentMagistralLiters - unauthorizedFlowLastMagistralLiters_) -
        (currentLeafLiters - unauthorizedFlowLastLeafLiters_);

    unauthorizedFlowLastMagistralLiters_ = currentMagistralLiters;
    unauthorizedFlowLastLeafLiters_ = currentLeafLiters;

    if (
        unauthorizedFlowWateringWorked_ ||
        unauthorizedWaterUsageLiters <= WATER_COUNTER_TOLERANCE_LITERS)
    {
        unauthorizedFlowWateringWorked_ = false;
        unauthorizedFlowDetected_ = false;
        unauthorizedFlowErrorLogged_ = false;
        return;
    }

    if (!unauthorizedFlowDetected_)
    {
        Logger::w(
            "Hypervisor",
            "Unauthorized water flow detected while watering is off: %.2f liters are not accounted for by leaf counters. Closing all valves",
            unauthorizedWaterUsageLiters);

        closeAllValves();
        unauthorizedFlowDetectedTimeMs_ = millis();
        unauthorizedFlowDetected_ = true;
        return;
    }

    if (!unauthorizedFlowErrorLogged_ &&
        now - unauthorizedFlowDetectedTimeMs_ >= UNAUTHORIZED_FLOW_ERROR_DELAY_MS)
    {
        Logger::e(
            "Hypervisor",
            "Unauthorized water flow persists after closing all valves: %.2f liters are not accounted for by leaf counters",
            unauthorizedWaterUsageLiters);

        unauthorizedFlowErrorLogged_ = true;
    }
}

void Hypervisor::checkOpenValvesWithoutWaterFlow()
{
    const WaterCounter *magistralCounter = waterHub_.getMagistralWaterCounter();
    if (magistralCounter == nullptr)
    {
        openValveWithoutFlowDetected_ = false;
        openValveWithoutFlowErrorLogged_ = false;
        return;
    }

    const uint32_t now = millis();
    if (now - openValveWithoutFlowLastCheckTimeMs_ < settings_.globalSettings.wateringWaterCounterReadIntervalSeconds * 1000)
    {
        return;
    }

    openValveWithoutFlowLastCheckTimeMs_ = now;
    const float currentMagistralLiters = magistralCounter->getTotalLiters();
    const float magistralUsageLiters = currentMagistralLiters - openValveWithoutFlowLastMagistralLiters_;
    openValveWithoutFlowLastMagistralLiters_ = currentMagistralLiters;

    const bool hasMagistralWaterFlow = magistralUsageLiters > WATER_COUNTER_TOLERANCE_LITERS;
    if (!waterHub_.hasOpenValve() || hasMagistralWaterFlow)
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
        now - openValveWithoutFlowDetectedTimeMs_ >= OPEN_VALVE_WITHOUT_FLOW_ERROR_DELAY_MS)
    {
        Logger::e(
            "Hypervisor",
            "At least one valve has been open for more than 10 seconds, but the magistral water counter shows no flow");

        openValveWithoutFlowErrorLogged_ = true;
    }
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
