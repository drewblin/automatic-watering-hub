#pragma once

#include <cmath>
#include <stdint.h>
#include "ModbusMaster.h"
#include "Setting/PressureSensorSetting.hpp"

class PressureSensor
{
public:
    PressureSensor(ModbusMaster &modbusNode,
                  HardwareSerial &serialPort,
                  const PressureSensorSetting &setting,
                  uint32_t readIntervalSeconds);
    void readPressureIfDue(uint32_t currentTimeMs);
    void readPressure();
    void setReadIntervalSeconds(uint32_t readIntervalSeconds);
    uint32_t getReadIntervalSeconds() const;
    float getLastReadPressure() const;

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    PressureSensorSetting setting_;
    uint32_t readIntervalSeconds_;
    uint32_t lastReadTimeMs_ = 0;
    float lastReadPressure_ = NAN;

    float convertToBar(float value, uint16_t unit);
};
