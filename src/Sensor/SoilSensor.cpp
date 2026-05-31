#include "SoilSensor.hpp"
#include <stdint.h>

SoilSensor::SoilSensor(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort,
    uint8_t slaveAddress,
    uint32_t readIntervalSeconds) : modbusNode_(modbusNode),
                                    serialPort_(serialPort),
                                    slaveAddress_(slaveAddress),
                                    readIntervalSeconds_(readIntervalSeconds)
{
}

void SoilSensor::readDataIfDue(uint32_t currentTimeMs)
{
    if (currentTimeMs - lastReadTimeMs_ < readIntervalSeconds_ * 1000)
    {
        return;
    }

    lastReadTimeMs_ = currentTimeMs;
    readData();
}

void SoilSensor::readData()
{
    modbusNode_.begin(slaveAddress_, serialPort_);

    uint8_t result = modbusNode_.readHoldingRegisters(0x0000, 2);
    if (result != modbusNode_.ku8MBSuccess)
    {
        ESP_LOGE(
            "SoilSensor",
            "Slave address %u returns mobdus result 0x%u",
            slaveAddress_,
            result);

        lastReadHumidity_ = NAN;
        lastReadTemperature_ = NAN;
        ++readingRevision_;

        return;
    }

    int16_t temperatureRaw = (int16_t)modbusNode_.getResponseBuffer(0);
    uint16_t humidityRaw = modbusNode_.getResponseBuffer(1);

    lastReadTemperature_ = temperatureRaw / 10.0f;
    lastReadHumidity_ = humidityRaw / 10.0f;
    ++readingRevision_;
}

void SoilSensor::setReadIntervalSeconds(uint32_t readIntervalSeconds)
{
    readIntervalSeconds_ = readIntervalSeconds;
}

uint32_t SoilSensor::getReadIntervalSeconds() const
{
    return readIntervalSeconds_;
}

float SoilSensor::getLastReadHumidity() const
{
    return lastReadHumidity_;
}

float SoilSensor::getLastReadTemperature() const
{
    return lastReadTemperature_;
}

uint32_t SoilSensor::getReadingRevision() const
{
    return readingRevision_;
}
