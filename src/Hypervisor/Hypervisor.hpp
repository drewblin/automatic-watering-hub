#pragma once

#include "Setting/SettingsSnapshot.hpp"
#include "Hub/WaterHub.hpp"

class Hypervisor
{
public:
    explicit Hypervisor(const SettingsSnapshot &settings, WaterHub &waterHub);

    void begin();
    void loop();

private:
    static constexpr float WATER_COUNTER_TOLERANCE_LITERS = 0.1f;
    static constexpr uint32_t UNAUTHORIZED_FLOW_ERROR_DELAY_MS = 10000;
    static constexpr uint32_t OPEN_VALVE_WITHOUT_FLOW_ERROR_DELAY_MS = 10000;

    const SettingsSnapshot settings_;
    WaterHub &waterHub_;

    bool unauthorizedFlowWateringWorked_ = false;
    uint32_t unauthorizedFlowLastCheckTimeMs_ = 0;
    float unauthorizedFlowLastMagistralLiters_ = 0;
    float unauthorizedFlowLastLeafLiters_ = 0;
    uint32_t unauthorizedFlowDetectedTimeMs_ = 0;
    bool unauthorizedFlowDetected_ = false;
    bool unauthorizedFlowErrorLogged_ = false;

    uint32_t openValveWithoutFlowLastCheckTimeMs_ = 0;
    float openValveWithoutFlowLastMagistralLiters_ = 0;
    uint32_t openValveWithoutFlowDetectedTimeMs_ = 0;
    bool openValveWithoutFlowDetected_ = false;
    bool openValveWithoutFlowErrorLogged_ = false;

    void checkUnauthorizedWaterFlow();
    void checkOpenValvesWithoutWaterFlow();
    void closeAllValves();
    void closeExpiredValves();
};
