#pragma once

#include <cmath>
#include <stdint.h>
#include "ModbusMaster.h"

class PressureSensor
{
public:
    PressureSensor(ModbusMaster &modbusNode,
                  HardwareSerial &serialPort,
                  uint8_t slaveAddress,
                  uint32_t readIntervalSeconds);
    void readPressureIfDue(uint32_t currentTimeMs);
    void readPressure();
    void setReadIntervalSeconds(uint32_t readIntervalSeconds);
    uint32_t getReadIntervalSeconds() const;
    float getLastReadPressure() const;
    uint32_t getReadingRevision() const;

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;
    uint32_t readIntervalSeconds_;
    uint32_t lastReadTimeMs_ = 0;
    float lastReadPressure_ = NAN;
    uint32_t readingRevision_ = 0;

    float convertToBar(float value, uint16_t unit);
};
