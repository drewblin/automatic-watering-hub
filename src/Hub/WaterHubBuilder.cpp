#include <unordered_map>
#include <memory>
#include "WaterHubBuilder.hpp"

WaterHubBuilder::WaterHubBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort) : modbusNode_(modbusNode),
                                        modbusSerialPort_(modbusSerialPort)
{
}

WaterHub WaterHubBuilder::build(Settings settings)
{
    GlobalSettings globalSettings = settings.getGlobalSettings();
    WaterHub waterHub;

    auto magistralWaterCounterSetting = settings.getMagistralWaterCounterSetting();
    auto magistralWaterCounter = std::make_unique<WaterCounter>(
        magistralWaterCounterSetting.getPin(),
        magistralWaterCounterSetting.getLitersPerTick(),
        globalSettings.waterCounterReadIntervalSeconds);
    waterHub.setMagistralWaterCounter(std::move(magistralWaterCounter));

    for (WaterCounterSetting setting : settings.getLeafWaterCounterSetting())
    {
        auto leafWaterCounter = std::make_unique<WaterCounter>(
            setting.getPin(),
            setting.getLitersPerTick(),
            globalSettings.waterCounterReadIntervalSeconds);
        waterHub.addLeafWaterCounter(std::move(leafWaterCounter));
    }

    auto pressureSensor = std::make_unique<PressureSensor>(
        modbusNode_,
        modbusSerialPort_,
        settings.getPressureSensorSetting().getSlaveAddress(),
        globalSettings.pressureSensorReadIntervalSeconds);
    waterHub.setPressureSensor(std::move(pressureSensor));

    std::unordered_map<std::uint8_t, SoilSensor *> soilSensorMap;
    for (SoilSensorSetting setting : settings.getSoilSensorSetting())
    {
        auto soilSensor = std::make_unique<SoilSensor>(
            modbusNode_,
            modbusSerialPort_,
            setting.getSlaveAddress(),
            globalSettings.soilSensorReadIntervalSeconds);
        SoilSensor *soilSensorPtr = soilSensor.get();

        waterHub.addSoilSensor(std::move(soilSensor));

        soilSensorMap[setting.getSlaveAddress()] = soilSensorPtr;
    }

    for (ValveSetting setting : settings.getValveSetting())
    {
        auto soilSensorIt = soilSensorMap.find(setting.getSoilSensorSlaveAddress());
        if (soilSensorIt == soilSensorMap.end())
        {
            ESP_LOGE(
                "WaterHubBuilder",
                "Valve pin %u references missing soil sensor address %u",
                setting.getPin(),
                setting.getSoilSensorSlaveAddress());

            continue;
        }

        auto valve = std::make_unique<Valve>(setting.getPin());
        waterHub.addValve(
            std::move(valve),
            soilSensorIt->second);
    }

    return waterHub;
}
