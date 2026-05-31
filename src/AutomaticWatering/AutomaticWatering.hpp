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
    uint32_t magistralWaterCounterReadingRevision_ = 0;
    uint32_t pressureSensorReadingRevision_ = 0;
    std::vector<uint32_t> leafWaterCounterReadingRevisions_;
    std::vector<uint32_t> soilSensorReadingRevisions_;

    void processMagistralWaterCounter();
    void processLeafWaterCounters();
    void processPressureSensor();
    void processSoilSensors();
};
