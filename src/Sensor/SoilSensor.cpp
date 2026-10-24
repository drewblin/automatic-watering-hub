#include "SoilSensor.hpp"
#include <stdint.h>
#include "Logging/Logger.hpp"
#include "Modbus/ModbusTransaction.hpp"

SoilSensor::SoilSensor(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort,
    ModbusSensorPower &power,
    const SoilSensorSetting &setting,
    uint32_t readIntervalSeconds) : modbusNode_(modbusNode),
                                    serialPort_(serialPort),
                                    power_(power),
                                    setting_(setting),
                                    readIntervalSeconds_(readIntervalSeconds)
{
}

bool SoilSensor::isReadDue(uint32_t currentTimeMs) const
{
    return currentTimeMs - lastReadTimeMs_ >= readIntervalSeconds_ * 1000;
}

void SoilSensor::readDataIfDue(uint32_t currentTimeMs)
{
    if (!isReadDue(currentTimeMs))
    {
        return;
    }
    lastReadTimeMs_ = currentTimeMs;
    readData();
}

void SoilSensor::readData()
{
    uint8_t result = modbusNode_.ku8MBSuccess;
    int16_t temperatureRaw = 0;
    uint16_t humidityRaw = 0;
    {
        ModbusTransaction transaction(power_);
        modbusNode_.begin(setting_.getSlaveAddress(), serialPort_);
        result = modbusNode_.readHoldingRegisters(0x0000, 2);
        if (result == modbusNode_.ku8MBSuccess)
        {
            temperatureRaw = (int16_t)modbusNode_.getResponseBuffer(0);
            humidityRaw = modbusNode_.getResponseBuffer(1);
        }
    }

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
