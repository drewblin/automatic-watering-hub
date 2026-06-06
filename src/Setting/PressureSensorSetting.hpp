#pragma once

#include <stdint.h>
#include <string>

class PressureSensorSetting
{
public:
    PressureSensorSetting(uint8_t slaveAddress, std::string name = "Pressure sensor");
    uint8_t getSlaveAddress() const;
    std::string getName() const;

private:
    uint8_t slaveAddress_;
    std::string name_;
};
