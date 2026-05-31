#pragma once

#include <memory>
#include "ApiServer.hpp"
#include "ModbusMaster.h"

class ApiServerBuilder
{
public:
    ApiServerBuilder(
        ModbusMaster &modbusNode,
        HardwareSerial &modbusSerialPort,
        Settings &settings);

    std::unique_ptr<ApiServer> build();

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
    Settings &settings_;
};
