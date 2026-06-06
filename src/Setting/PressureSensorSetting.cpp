#include "PressureSensorSetting.hpp"

#include <utility>

PressureSensorSetting::PressureSensorSetting(uint8_t slaveAddress, std::string name)
    : slaveAddress_(slaveAddress), name_(std::move(name))
{
}

uint8_t PressureSensorSetting::getSlaveAddress() const
{
    return slaveAddress_;
}

std::string PressureSensorSetting::getName() const
{
    return name_;
}
