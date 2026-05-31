#include "PressureSensorSetting.hpp"

PressureSensorSetting::PressureSensorSetting(uint8_t slaveAddress)
    : slaveAddress_(slaveAddress)
{
}

uint8_t PressureSensorSetting::getSlaveAddress() const
{
    return slaveAddress_;
}
