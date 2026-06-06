#pragma once

#include <vector>
#include <memory>
#include <unordered_map>
#include "Sensor/WaterCounter.hpp"
#include "Sensor/PressureSensor.hpp"
#include "Sensor/SoilSensor.hpp"
#include "Setting/GlobalSettings.hpp"
#include "Valve/Valve.hpp"

class WaterHub
{
public:
    explicit WaterHub(const GlobalSettings &globalSettings);

    void setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter);
    void setPressureSensor(std::unique_ptr<PressureSensor> sensor);
    void addLeafWaterCounter(std::unique_ptr<WaterCounter> counter);
    void addSoilSensor(std::unique_ptr<SoilSensor> sensor);
    void addValve(std::unique_ptr<Valve> valve, SoilSensor *sensor);

    void loop();

    const WaterCounter *getMagistralWaterCounter() const;
    const PressureSensor *getPressureSensor() const;
    const std::vector<std::unique_ptr<WaterCounter>> &getLeafWaterCounters() const;
    const std::vector<std::unique_ptr<SoilSensor>> &getSoilSensors() const;
    const std::vector<std::unique_ptr<Valve>> &getValves() const;
    SoilSensor *getSoilSensorForValve(const Valve *valve) const;

    bool hasOpenValve() const;

private:
    std::unique_ptr<WaterCounter> magistralWaterCounter_;
    std::unique_ptr<PressureSensor> pressureSensor_;

    std::vector<std::unique_ptr<WaterCounter>> leafWaterCounters_;
    std::vector<std::unique_ptr<SoilSensor>> soilSensors_;
    std::vector<std::unique_ptr<Valve>> valves_;

    std::unordered_map<const Valve *, SoilSensor *> valveToSoilSensorMap_;

    GlobalSettings globalSettings_;
    bool sensorReadIntervalsUseWateringMode_ = false;

    void updateSensorReadIntervals();
    void readWaterCounters(uint32_t currentTimeMs);
    void readPressureSensor(uint32_t currentTimeMs);
    void readSoilSensors(uint32_t currentTimeMs);
};
