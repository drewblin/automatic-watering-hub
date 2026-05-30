#pragma once

#include "ApiServer.hpp"
#include "Hub/WaterHub.hpp"
#include "ModbusMaster.h"

class ApiServerBuilder
{
public:
    ApiServerBuilder(
        ModbusMaster &modbusNode,
        HardwareSerial &modbusSerialPort,
        WaterHub &waterHub,
        Settings &settings);

    ApiServer build();

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
    WaterHub &waterHub_;
    Settings &settings_;
};
