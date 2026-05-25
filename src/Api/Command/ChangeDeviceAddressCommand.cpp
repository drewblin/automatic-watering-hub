#include "ChangeDeviceAddressCommand.hpp"

ChangeDeviceAddressCommand::ChangeDeviceAddressCommand(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort)
    : modbusNode_(modbusNode),
      serialPort_(serialPort)
{
}

uint8_t ChangeDeviceAddressCommand::execute(uint8_t currentAddress, uint8_t newAddress, uint16_t registerAddress)
{
    modbusNode_.begin(currentAddress, serialPort_);
    return modbusNode_.writeSingleRegister(registerAddress, newAddress);
}
