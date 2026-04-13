#include "ValveSetting.hpp"

ValveSetting::ValveSetting(uint8_t pin, std::string name, uint8_t soilSensorSlaveAddress)
    : pin_(pin), name_(name), soilSensorSlaveAddress_(soilSensorSlaveAddress)
{
}

uint8_t ValveSetting::getPin()
{
    return pin_;
}

std::string ValveSetting::getName()
{
    return name_;
}

uint8_t ValveSetting::getSoilSensorSlaveAddress()
{
    return soilSensorSlaveAddress_;
}