#pragma once

#include <cmath>
#include <stdint.h>
#include "ModbusMaster.h"

class PressureSensor
{
public:
    PressureSensor(ModbusMaster &modbusNode,
                  HardwareSerial &serialPort,
                  uint8_t slaveAddress);
    void readPressure();
    float getLastReadPressure() const;
    uint32_t getReadingRevision() const;

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;
    float lastReadPressure_ = NAN;
    uint32_t readingRevision_ = 0;

    float convertToBar(float value, uint16_t unit);
};
