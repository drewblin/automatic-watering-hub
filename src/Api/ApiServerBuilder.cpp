#include "ApiServerBuilder.hpp"

#include "Command/ChangeDeviceAddressCommand.hpp"

ApiServerBuilder::ApiServerBuilder(ModbusMaster &modbusNode, HardwareSerial &modbusSerialPort)
    : modbusNode_(modbusNode),
      modbusSerialPort_(modbusSerialPort)
{
}

ApiServer ApiServerBuilder::build()
{
    return ApiServer(ChangeDeviceAddressCommand(modbusNode_, modbusSerialPort_));
}
