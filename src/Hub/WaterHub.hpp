#include <vector>
#include <memory>
#include <unordered_map>
#include "Sensor/WaterCounter.hpp"
#include "Sensor/PresureSensor.hpp"
#include "Sensor/HumiditySensor.hpp"
#include "Valve/Valve.hpp"

class WaterHub
{
public:
    WaterHub() = default;

    void setMagistralWaterCounter(std::unique_ptr<WaterCounter> counter);
    void setPresureSensor(std::unique_ptr<PresureSendor> sensor);
    void addLeafWaterCounter(std::unique_ptr<WaterCounter> counter);
    void addHumiditySensor(std::unique_ptr<HumiditySensor> sensor);
    void addValve(std::unique_ptr<Valve> valve, HumiditySensor* sensor);

private:
    std::unique_ptr<WaterCounter> magistralWaterCounter;
    std::unique_ptr<PresureSendor> presureSensor;

    std::vector<std::unique_ptr<WaterCounter>> leafWaterCounters;
    std::vector<std::unique_ptr<HumiditySensor>> humiditySensors;
    std::vector<std::unique_ptr<Valve>> valves;

    std::unordered_map<Valve*, HumiditySensor*> valveToHumiditySensorMap;
};
