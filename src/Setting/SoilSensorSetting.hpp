#pragma once

#include <stdint.h>
#include <string>

class SoilSensorSetting
{
public:
    SoilSensorSetting(uint8_t slaveAddress, std::string name);
    uint8_t getSlaveAddress() const;
    std::string getName() const;

private:
    uint8_t slaveAddress_;
    std::string name_;
};
