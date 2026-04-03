#include <vector>
#include <memory>
#include <unordered_map>
#include "Sensor/WaterCounter.hpp"
#include "Sensor/PresureSensor.hpp"
#include "Sensor/SoilSensor.hpp"
#include "Valve/Valve.hpp"

class WaterHub
{
public:
    WaterHub() = default;

    void setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter);
    void setPresureSensor(std::unique_ptr<PresureSendor> sensor);
    void addLeafWaterCounter(std::unique_ptr<WaterCounter> counter);
    void addSoilSensor(std::unique_ptr<SoilSensor> sensor);
    void addValve(std::unique_ptr<Valve> valve, SoilSensor* sensor);

private:
    std::unique_ptr<WaterCounter> magistralWaterCounter;
    std::unique_ptr<PresureSendor> presureSensor;

    std::vector<std::unique_ptr<WaterCounter>> leafWaterCounters;
    std::vector<std::unique_ptr<SoilSensor>> soilSensors;
    std::vector<std::unique_ptr<Valve>> valves;

    std::unordered_map<Valve*, SoilSensor*> valveToSoilSensorMap;
};
