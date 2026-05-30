#include "ApiServerBuilder.hpp"

#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"

ApiServerBuilder::ApiServerBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort,
    WaterHub &waterHub,
    Settings &settings)
    : modbusNode_(modbusNode),
      modbusSerialPort_(modbusSerialPort),
      waterHub_(waterHub),
      settings_(settings)
{
}

ApiServer ApiServerBuilder::build()
{
    return ApiServer(
        ChangeDeviceAddressCommand(modbusNode_, modbusSerialPort_),
        OpenValveForTimeCommand(waterHub_),
        settings_);
}
