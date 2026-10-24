#pragma once

#include <cmath>
#include <stdint.h>
#include "ModbusMaster.h"
#include "Modbus/ModbusSensorPower.hpp"
#include "Setting/SoilSensorSetting.hpp"

class SoilSensor
{
public:
    SoilSensor(
        ModbusMaster &modbusNode,
        HardwareSerial &serialPort,
        ModbusSensorPower &power,
        const SoilSensorSetting &setting,
        uint32_t readIntervalSeconds);
    bool isReadDue(uint32_t currentTimeMs) const;
    void readDataIfDue(uint32_t currentTimeMs);
    void readData();
    void setReadIntervalSeconds(uint32_t readIntervalSeconds);
    uint32_t getReadIntervalSeconds() const;
    float getLastReadHumidity() const;
    float getLastReadTemperature() const;

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    ModbusSensorPower &power_;
    SoilSensorSetting setting_;
    uint32_t readIntervalSeconds_;
    uint32_t lastReadTimeMs_ = 0;
    float lastReadHumidity_ = NAN;
    float lastReadTemperature_ = NAN;
};
