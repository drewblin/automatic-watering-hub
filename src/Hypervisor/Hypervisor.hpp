#pragma once

#include "Hub/WaterHub.hpp"

class Hypervisor
{
public:
    explicit Hypervisor(WaterHub &waterHub);

    void begin();
    void loop();

private:
    static constexpr float WATER_COUNTER_TOLERANCE_LITERS = 0.1f;
    static constexpr uint32_t UNAUTHORIZED_FLOW_ERROR_DELAY_MS = 10000;

    WaterHub &waterHub_;
    uint32_t magistralWaterCounterReadingRevision_ = 0;
    uint32_t unauthorizedFlowDetectedTimeMs_ = 0;
    bool unauthorizedFlowDetected_ = false;
    bool unauthorizedFlowErrorLogged_ = false;

    void checkUnauthorizedWaterFlow();
    void closeAllValves();
    void closeExpiredValves();
};
