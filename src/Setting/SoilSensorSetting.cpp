#include "SoilSensorSetting.hpp"

SoilSensorSetting::SoilSensorSetting(uint8_t slaveAddress, std::string name)
    : slaveAddress_(slaveAddress), name_(name)
{
}

uint8_t SoilSensorSetting::getSlaveAddress()
{
    return slaveAddress_;
}

std::string SoilSensorSetting::getName()
{
    return name_;
}