#pragma once

#include "ModbusMaster.h"

class ChangeDeviceAddressCommand
{
public:
    static const uint8_t SuccessStatus = ModbusMaster::ku8MBSuccess;

    ChangeDeviceAddressCommand(ModbusMaster &modbusNode, HardwareSerial &serialPort);

    uint8_t execute(
        uint8_t currentAddress,
        uint8_t newAddress,
        uint16_t registerAddress,
        bool save,
        uint16_t saveRegisterAddress,
        uint16_t saveValue);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
};
