#pragma once

#include <cmath>
#include <stdint.h>
#include "ModbusMaster.h"

class SoilSensor
{
public:
    SoilSensor(
        ModbusMaster &modbusNode,
        HardwareSerial &serialPort,
        uint8_t slaveAddress,
        uint32_t readIntervalSeconds);
    void readDataIfDue(uint32_t currentTimeMs);
    void readData();
    void setReadIntervalSeconds(uint32_t readIntervalSeconds);
    uint32_t getReadIntervalSeconds() const;
    float getLastReadHumidity() const;
    float getLastReadTemperature() const;

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;
    uint32_t readIntervalSeconds_;
    uint32_t lastReadTimeMs_ = 0;
    float lastReadHumidity_ = NAN;
    float lastReadTemperature_ = NAN;
};
