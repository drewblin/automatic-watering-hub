#include "SoilSensor.hpp"
#include <stdint.h>
#include "Logging/Logger.hpp"

SoilSensor::SoilSensor(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort,
    const SoilSensorSetting &setting,
    uint32_t readIntervalSeconds) : modbusNode_(modbusNode),
                                    serialPort_(serialPort),
                                    setting_(setting),
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
    modbusNode_.begin(setting_.getSlaveAddress(), serialPort_);

    uint8_t result = modbusNode_.readHoldingRegisters(0x0000, 2);
    if (result != modbusNode_.ku8MBSuccess)
    {
        Logger::e(
            "SoilSensor",
            "Slave address %u returns mobdus result 0x%u",
            setting_.getSlaveAddress(),
            result);

        lastReadHumidity_ = NAN;
        lastReadTemperature_ = NAN;
        Logger::sendSensorReading(setting_.getSlaveAddress(), "soil_temperature", setting_.getName().c_str(), lastReadTemperature_);
        Logger::sendSensorReading(setting_.getSlaveAddress(), "soil_humidity", setting_.getName().c_str(), lastReadHumidity_);

        return;
    }

    int16_t temperatureRaw = (int16_t)modbusNode_.getResponseBuffer(0);
    uint16_t humidityRaw = modbusNode_.getResponseBuffer(1);

    lastReadTemperature_ = temperatureRaw / 10.0f;
    lastReadHumidity_ = humidityRaw / 10.0f;
    Logger::sendSensorReading(setting_.getSlaveAddress(), "soil_temperature", setting_.getName().c_str(), lastReadTemperature_);
    Logger::sendSensorReading(setting_.getSlaveAddress(), "soil_humidity", setting_.getName().c_str(), lastReadHumidity_);
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
