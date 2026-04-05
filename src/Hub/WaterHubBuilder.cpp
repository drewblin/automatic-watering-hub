#include <unordered_map>
#include <memory>
#include "WaterHubBuilder.hpp"

WaterHubBuilder::WaterHubBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort) : modbusNode(modbusNode),
                                        modbusSerialPort(modbusSerialPort)
{
}

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
        modbusNode,
        modbusSerialPort,
        settings.getPresureSensorSetting().getSlaveAddress());
    waterHub.setPresureSensor(std::move(presureSensor));

    std::unordered_map<std::uint8_t, SoilSensor *> soilSensorMap;
    for (SoilSensorSetting setting : settings.getSoilSensorSetting())
    {
        auto soilSensor = std::make_unique<SoilSensor>(
            modbusNode,
            modbusSerialPort,
            setting.getSlaveAddress());
        SoilSensor *soilSensorPtr = soilSensor.get();

        waterHub.addSoilSensor(std::move(soilSensor));

        soilSensorMap[setting.getSlaveAddress()] = soilSensorPtr;
    }

    for (ValveSetting setting : settings.getValveSetting())
    {
        auto valve = std::make_unique<Valve>(setting.getPin());
        waterHub.addValve(
            std::move(valve),
            soilSensorMap.find(setting.getSoilSensorSlaveAddress())->second);
    }

    return waterHub;
}
