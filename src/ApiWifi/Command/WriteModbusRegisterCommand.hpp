#pragma once

#include "Api/ApiCommandResult.hpp"
#include "ModbusMaster.h"

class WriteModbusRegisterCommand
{
public:
    static const uint8_t SuccessStatus = ModbusMaster::ku8MBSuccess;

    WriteModbusRegisterCommand(ModbusMaster &modbusNode, HardwareSerial &serialPort);
    ApiCommandResult execute(const JsonDocument &request);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
};
