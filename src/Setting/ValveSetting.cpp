#include "ValveSetting.hpp"

ValveSetting::ValveSetting(uint8_t pin, std::string name, uint8_t soilSensorSlaveAddress)
    : pin_(pin), name_(name), soilSensorSlaveAddress_(soilSensorSlaveAddress)
{
}

uint8_t ValveSetting::getPin() const
{
    return pin_;
}

std::string ValveSetting::getName() const
{
    return name_;
}

uint8_t ValveSetting::getSoilSensorSlaveAddress() const
{
    return soilSensorSlaveAddress_;
}
