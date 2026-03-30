#include <unordered_map>
#include <memory>
#include "WaterHubBuilder.hpp"

WaterHub WaterHubBuilder::build(Settings settings)
{
    WaterHub waterHub = WaterHub();

    auto magistralWaterCounter = std::make_unique<WaterCounter>(
        settings.getMagistralWaterCounterSetting().getPin());
    waterHub.setMagistralWaterCounter(std::move(magistralWaterCounter));

    for (WaterCounterSetting setting : settings.getLeafWaterCounterSetting())
    {
        auto leafWaterCounter = std::make_unique<WaterCounter>(setting.getPin());
        waterHub.addLeafWaterCounter(std::move(leafWaterCounter));
    }

    auto presureSensor = std::make_unique<PresureSendor>(
        settings.getPresureSensorSetting().getSlaveAddress());
    waterHub.setPresureSensor(std::move(presureSensor));

    std::unordered_map<std::uint8_t, HumiditySensor *> humiditySensorMap;
    for (HumiditySensorSetting setting : settings.getHumiditySensorSetting())
    {
        auto humiditySensor = std::make_unique<HumiditySensor>(setting.getSlaveAddress());
        HumiditySensor *humiditySensorPtr = humiditySensor.get();

        waterHub.addHumiditySensor(std::move(humiditySensor));

        humiditySensorMap[setting.getSlaveAddress()] = humiditySensorPtr;
    }

    for (ValveSetting setting : settings.getValveSetting())
    {
        auto valve = std::make_unique<Valve>(setting.getPin());
        waterHub.addValve(
            std::move(valve),
            humiditySensorMap.find(setting.getHumiditySlaveAddress())->second);
    }

    return waterHub;
}
