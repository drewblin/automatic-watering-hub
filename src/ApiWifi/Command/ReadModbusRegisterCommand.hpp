#pragma once

#include "Api/ApiCommandResult.hpp"
#include "ModbusMaster.h"
#include "Modbus/ModbusSensorPower.hpp"

class ReadModbusRegisterCommand
{
public:
    static const uint8_t SuccessStatus = ModbusMaster::ku8MBSuccess;
    static const uint16_t MaxRegisterQuantity = 64;

    ReadModbusRegisterCommand(ModbusMaster &modbusNode, HardwareSerial &serialPort, ModbusSensorPower &power);
    ApiCommandResult execute(const JsonDocument &request);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    ModbusSensorPower &power_;
};
