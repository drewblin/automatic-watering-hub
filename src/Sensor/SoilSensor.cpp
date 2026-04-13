#include "SoilSensor.hpp"
#include <stdint.h>

SoilSensor::SoilSensor(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort,
    uint8_t slaveAddress) : modbusNode_(modbusNode),
                            serialPort_(serialPort),
                            slaveAddress_(slaveAddress)
{
}

void SoilSensor::readData(float &humidity, float &temperature)
{
    modbusNode_.begin(slaveAddress_, serialPort_);

    uint8_t result = modbusNode_.readHoldingRegisters(0x0000, 2);
    if (result != modbusNode_.ku8MBSuccess)
    {
        humidity = NAN;
        temperature = NAN;

        return;
    }

    int16_t temperatureRaw = (int16_t)modbusNode_.getResponseBuffer(0);
    uint16_t humidityRaw = modbusNode_.getResponseBuffer(1);

    temperature = temperatureRaw / 10.0f;
    humidity = humidityRaw / 10.0f;
}
