#include "ChangeDeviceAddressCommand.hpp"

#include "Api/JsonRequestReader.hpp"

ChangeDeviceAddressCommand::ChangeDeviceAddressCommand(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort)
    : modbusNode_(modbusNode),
      serialPort_(serialPort)
{
}

ApiCommandResult ChangeDeviceAddressCommand::execute(const JsonDocument &request)
{
    ApiCommandResult result(400, false);
    uint8_t currentAddress = 0;
    uint8_t newAddress = 0;
    uint16_t registerAddress = 0;
    uint16_t saveRegisterAddress = 0;
    uint16_t saveValue = 0;
    bool hasSaveRegisterAddress = false;
    bool hasSaveValue = false;
    JsonVariantConst json = request.as<JsonVariantConst>();
    if (!JsonRequestReader::readRequiredUint8(json, "currentAddress", currentAddress, result.error) ||
        !JsonRequestReader::readRequiredUint8(json, "newAddress", newAddress, result.error) ||
        !JsonRequestReader::readRequiredUint16(json, "registerAddress", registerAddress, result.error) ||
        !JsonRequestReader::readOptionalUint16(json, "saveRegisterAddress", saveRegisterAddress, hasSaveRegisterAddress, result.error) ||
        !JsonRequestReader::readOptionalUint16(json, "saveValue", saveValue, hasSaveValue, result.error))
    {
        return result;
    }
    if (hasSaveRegisterAddress != hasSaveValue)
    {
        result.error = "saveRegisterAddress and saveValue must be provided together";
        return result;
    }

    bool save = hasSaveRegisterAddress && hasSaveValue;
    uint8_t status = execute(currentAddress, newAddress, registerAddress, save, saveRegisterAddress, saveValue);
    result.data["status"] = status;
    result.data["currentAddress"] = currentAddress;
    result.data["newAddress"] = newAddress;
    result.data["registerAddress"] = registerAddress;
    result.data["save"] = save;
    if (save)
    {
        result.data["saveRegisterAddress"] = saveRegisterAddress;
        result.data["saveValue"] = saveValue;
    }
    if (status != SuccessStatus)
    {
        result.statusCode = 502;
        result.error = "Failed to change device address";
        return result;
    }

    result.statusCode = 200;
    result.success = true;
    return result;
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
