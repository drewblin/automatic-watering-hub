#pragma once

#include "ApiServer.hpp"
#include "ModbusMaster.h"

class ApiServerBuilder
{
public:
    ApiServerBuilder(ModbusMaster &modbusNode, HardwareSerial &modbusSerialPort);

    ApiServer build();

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
};
