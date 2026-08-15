#include <unordered_map>
#include <memory>
#include "WaterHubBuilder.hpp"
#include "Logging/Logger.hpp"

WaterHubBuilder::WaterHubBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort) : modbusNode_(modbusNode),
                                        modbusSerialPort_(modbusSerialPort)
{
}

WaterHub WaterHubBuilder::build(const SettingsSnapshot &settings)
{
    GlobalSettings globalSettings = settings.globalSettings;
    WaterHub waterHub(globalSettings);

    if (settings.magistralWaterCounterSetting.has_value())
    {
        auto magistralWaterCounter = std::make_unique<WaterCounter>(
            settings.magistralWaterCounterSetting.value(),
            globalSettings.idleWaterCounterReadIntervalSeconds);
        waterHub.setMagistralWaterCounter(std::move(magistralWaterCounter));
    }

    for (WaterCounterSetting setting : settings.leafWaterCounterSettings)
    {
        auto leafWaterCounter = std::make_unique<WaterCounter>(
            setting,
            globalSettings.idleWaterCounterReadIntervalSeconds);
        waterHub.addLeafWaterCounter(std::move(leafWaterCounter));
    }

    if (settings.pressureSensorSetting.has_value())
    {
        auto pressureSensor = std::make_unique<PressureSensor>(
            modbusNode_,
            modbusSerialPort_,
            settings.pressureSensorSetting.value(),
            globalSettings.idlePressureSensorReadIntervalSeconds);
        waterHub.setPressureSensor(std::move(pressureSensor));
    }

    std::unordered_map<std::uint8_t, SoilSensor *> soilSensorMap;
    for (SoilSensorSetting setting : settings.soilSensorSettings)
    {
        auto soilSensor = std::make_unique<SoilSensor>(
            modbusNode_,
            modbusSerialPort_,
            setting,
            globalSettings.idleSoilSensorReadIntervalSeconds);
        SoilSensor *soilSensorPtr = soilSensor.get();

        waterHub.addSoilSensor(std::move(soilSensor));

        soilSensorMap[setting.getSlaveAddress()] = soilSensorPtr;
    }

    for (ValveSetting setting : settings.valveSettings)
    {
        auto soilSensorIt = soilSensorMap.find(setting.getSoilSensorSlaveAddress());
        if (soilSensorIt == soilSensorMap.end())
        {
            Logger::e(
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
