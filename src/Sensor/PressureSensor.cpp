#include "PressureSensor.hpp"
#include <stdint.h>
#include "Logging/Logger.hpp"

PressureSensor::PressureSensor(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort,
    const PressureSensorSetting &setting,
    uint32_t readIntervalSeconds) : modbusNode_(modbusNode),
                                    serialPort_(serialPort),
                                    setting_(setting),
                                    readIntervalSeconds_(readIntervalSeconds)
{
}

void PressureSensor::readPressureIfDue(uint32_t currentTimeMs)
{
    if (currentTimeMs - lastReadTimeMs_ < readIntervalSeconds_ * 1000)
    {
        return;
    }

    lastReadTimeMs_ = currentTimeMs;
    readPressure();
}

void PressureSensor::readPressure()
{
    modbusNode_.begin(setting_.getSlaveAddress(), serialPort_);

    uint8_t result = modbusNode_.readHoldingRegisters(0x0002, 3);
    if (result != modbusNode_.ku8MBSuccess)
    {
        Logger::e(
            "PressureSensor",
            "Slave address %u returns mobdus result 0x%u",
            setting_.getSlaveAddress(),
            result);

        lastReadPressure_ = NAN;
        Logger::sendSensorReading(setting_.getSlaveAddress(), "pressure", setting_.getName().c_str(), lastReadPressure_);
        return;
    }

    uint16_t unit = modbusNode_.getResponseBuffer(0);
    uint16_t decimal = modbusNode_.getResponseBuffer(1);
    int16_t pressureRaw = (int16_t)modbusNode_.getResponseBuffer(2);

    lastReadPressure_ = convertToBar(pressureRaw / powf(10, decimal), unit);
    Logger::sendSensorReading(setting_.getSlaveAddress(), "pressure", setting_.getName().c_str(), lastReadPressure_);
}

void PressureSensor::setReadIntervalSeconds(uint32_t readIntervalSeconds)
{
    readIntervalSeconds_ = readIntervalSeconds;
}

uint32_t PressureSensor::getReadIntervalSeconds() const
{
    return readIntervalSeconds_;
}

float PressureSensor::getLastReadPressure() const
{
    return lastReadPressure_;
}

float PressureSensor::convertToBar(float value, uint16_t unit)
{
    switch (unit)
    {
    case 0:
        return value * 10.0f; // MPa → bar
    case 1:
        return value / 100.0f; // kPa → bar
    case 2:
        return value / 100000.0f; // Pa → bar
    case 3:
        return value; // bar
    case 4:
        return value / 1000.0f; // mbar → bar
    case 5:
        return value * 0.980665f; // kg/cm² → bar
    case 6:
        return value * 0.0689476f; // PSI → bar
    case 7:
        return value * 0.0980665f; // mH2O → bar
    case 8:
        return value * 0.0000980665f; // mmH2O → bar
    case 9:
        return value * 0.00249089f; // inH2O → bar
    case 10:
        return value * 0.0980665f; // H2O (~mH2O) → bar
    case 11:
        return value * 1.33322f; // mHg → bar
    case 12:
        return value * 0.00133322f; // mmHg → bar
    case 13:
        return value * 0.0338639f; // inHg → bar
    case 14:
        return value * 1.01325f; // atm → bar
    case 15:
        return value * 0.00133322f; // Torr → bar

    default:
        return NAN;
    }
}
