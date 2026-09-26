#include "WriteModbusRegisterCommand.hpp"

#include "Api/JsonRequestReader.hpp"
#include "Modbus/ModbusBusLock.hpp"

WriteModbusRegisterCommand::WriteModbusRegisterCommand(
    ModbusMaster &modbusNode,
    HardwareSerial &serialPort)
    : modbusNode_(modbusNode),
      serialPort_(serialPort)
{
}

ApiCommandResult WriteModbusRegisterCommand::execute(const JsonDocument &request)
{
    ApiCommandResult result(400, false);
    uint8_t slaveAddress = 0;
    uint16_t registerAddress = 0;
    uint16_t value = 0;
    JsonVariantConst json = request.as<JsonVariantConst>();
    if (!JsonRequestReader::readRequiredUint8(json, "slaveAddress", slaveAddress, result.error) ||
        !JsonRequestReader::readRequiredUint16(json, "registerAddress", registerAddress, result.error) ||
        !JsonRequestReader::readRequiredUint16(json, "value", value, result.error))
    {
        return result;
    }

    uint8_t status;
    {
        ModbusBusLock lock;
        modbusNode_.begin(slaveAddress, serialPort_);
        status = modbusNode_.writeSingleRegister(registerAddress, value);
    }

    result.data["status"] = status;
    result.data["slaveAddress"] = slaveAddress;
    result.data["registerAddress"] = registerAddress;
    result.data["value"] = value;
    if (status != SuccessStatus)
    {
        result.statusCode = 502;
        result.error = "Failed to write Modbus register";
        return result;
    }

    result.statusCode = 200;
    result.success = true;
    return result;
}
