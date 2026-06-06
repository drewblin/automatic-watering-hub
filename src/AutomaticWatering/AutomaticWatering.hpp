#pragma once

#include <cstdint>
#include <vector>
#include "Hub/WaterHub.hpp"

class AutomaticWatering
{
public:
    explicit AutomaticWatering(const WaterHub &waterHub);

    void loop();

private:
    const WaterHub &waterHub_;

    void processMagistralWaterCounter();
    void processLeafWaterCounters();
    void processPressureSensor();
    void processSoilSensors();
};
