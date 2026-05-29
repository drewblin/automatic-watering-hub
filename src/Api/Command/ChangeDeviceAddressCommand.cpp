#include "ChangeDeviceAddressCommand.hpp"

ChangeDeviceAddressCommand::ChangeDeviceAddressCommand(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort)
    : modbusNode_(modbusNode),
      serialPort_(serialPort)
{
}

uint8_t ChangeDeviceAddressCommand::execute(
    uint8_t currentAddress,
    uint8_t newAddress,
    uint16_t registerAddress,
    bool save,
    uint16_t saveRegisterAddress,
    uint16_t saveValue)
{
    modbusNode_.begin(currentAddress, serialPort_);
    uint8_t status = modbusNode_.writeSingleRegister(registerAddress, newAddress);
    if (status != SuccessStatus || !save)
    {
        return status;
    }

    modbusNode_.begin(newAddress, serialPort_);
    return modbusNode_.writeSingleRegister(saveRegisterAddress, saveValue);
}
