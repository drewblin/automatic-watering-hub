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
        uint8_t slaveAddress);
    void readData();
    float getLastReadHumidity() const;
    float getLastReadTemperature() const;
    uint32_t getReadingRevision() const;

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;
    float lastReadHumidity_ = NAN;
    float lastReadTemperature_ = NAN;
    uint32_t readingRevision_ = 0;
};
