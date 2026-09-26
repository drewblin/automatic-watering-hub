#include "ReadModbusRegisterCommand.hpp"

#include "Api/JsonRequestReader.hpp"
#include "Modbus/ModbusBusLock.hpp"

ReadModbusRegisterCommand::ReadModbusRegisterCommand(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort)
    : modbusNode_(modbusNode),
      serialPort_(serialPort)
{
}

ApiCommandResult ReadModbusRegisterCommand::execute(const JsonDocument &request)
{
    ApiCommandResult result(400, false);
    uint8_t slaveAddress = 0;
    uint16_t registerAddress = 0;
    uint16_t quantity = 0;
    JsonVariantConst json = request.as<JsonVariantConst>();
    if (!JsonRequestReader::readRequiredUint8(json, "slaveAddress", slaveAddress, result.error) ||
        !JsonRequestReader::readRequiredUint16(json, "registerAddress", registerAddress, result.error) ||
        !JsonRequestReader::readRequiredUint16(json, "quantity", quantity, result.error))
    {
        return result;
    }
    if (quantity == 0 || quantity > MaxRegisterQuantity)
    {
        result.error = String("Quantity must be between 1 and ") + MaxRegisterQuantity;
        return result;
    }

    uint8_t status = SuccessStatus;
    uint16_t registerValues[MaxRegisterQuantity] = {0};
    {
        ModbusBusLock lock;
        modbusNode_.begin(slaveAddress, serialPort_);
        status = modbusNode_.readHoldingRegisters(registerAddress, quantity);
        if (status == SuccessStatus)
        {
            for (uint16_t i = 0; i < quantity; ++i)
            {
                registerValues[i] = modbusNode_.getResponseBuffer(i);
            }
        }
    }

    result.data["status"] = status;
    result.data["slaveAddress"] = slaveAddress;
    result.data["registerAddress"] = registerAddress;
    result.data["quantity"] = quantity;
    if (status != SuccessStatus)
    {
        result.statusCode = 502;
        result.error = "Failed to read Modbus register";
        return result;
    }

    JsonArray values = result.data["values"].to<JsonArray>();
    for (uint16_t i = 0; i < quantity; ++i)
    {
        values.add(registerValues[i]);
    }

    result.statusCode = 200;
    result.success = true;
    return result;
}
