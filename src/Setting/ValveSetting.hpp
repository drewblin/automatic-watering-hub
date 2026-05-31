#pragma once

#include <stdint.h>
#include <string>

class ValveSetting
{
public:
    ValveSetting(uint8_t pin, std::string name, uint8_t soilSensorSlaveAddress);
    uint8_t getPin() const;
    std::string getName() const;
    uint8_t getSoilSensorSlaveAddress() const;

private:
    uint8_t pin_;
    std::string name_;
    uint8_t soilSensorSlaveAddress_;
};
