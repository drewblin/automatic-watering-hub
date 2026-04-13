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
    WaterHub waterHub = WaterHub();

    auto magistralWaterCounterSetting = settings.getMagistralWaterCounterSetting();
    auto magistralWaterCounter = std::make_unique<WaterCounter>(
        magistralWaterCounterSetting.getPin(),
        magistralWaterCounterSetting.getLitersPerTick()
    );
    waterHub.setMagistralWaterCounter(std::move(magistralWaterCounter));

    for (WaterCounterSetting setting : settings.getLeafWaterCounterSetting())
    {
        auto leafWaterCounter = std::make_unique<WaterCounter>(
            setting.getPin(),
            setting.getLitersPerTick()
        );
        waterHub.addLeafWaterCounter(std::move(leafWaterCounter));
    }

    auto pressureSensor = std::make_unique<PressureSendor>(
        modbusNode_,
        modbusSerialPort_,
        settings.getPressureSensorSetting().getSlaveAddress());
    waterHub.setPressureSensor(std::move(pressureSensor));

    std::unordered_map<std::uint8_t, SoilSensor *> soilSensorMap;
    for (SoilSensorSetting setting : settings.getSoilSensorSetting())
    {
        auto soilSensor = std::make_unique<SoilSensor>(
            modbusNode_,
            modbusSerialPort_,
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
