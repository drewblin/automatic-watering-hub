#include "SoilSensor.hpp"
#include <stdint.h>

SoilSensor::SoilSensor(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort,
    uint8_t slaveAddress) : modbusNode(modbusNode),
                            serialPort(serialPort),
                            slaveAddress(slaveAddress)
{
}

void SoilSensor::readData(float &humidity, float &temperature)
{
    modbusNode.begin(slaveAddress, serialPort);

    uint8_t result = modbusNode.readHoldingRegisters(0x0000, 2);
    if (result != modbusNode.ku8MBSuccess)
    {
        humidity = NAN;
        temperature = NAN;

        return;
    }

    int16_t temperatureRaw = (int16_t)modbusNode.getResponseBuffer(0);
    uint16_t humidityRaw = modbusNode.getResponseBuffer(1);

    temperature = temperatureRaw / 10.0f;
    humidity = humidityRaw / 10.0f;
}
