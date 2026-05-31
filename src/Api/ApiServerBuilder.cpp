#include "ApiServerBuilder.hpp"

#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"

ApiServerBuilder::ApiServerBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort,
    Settings &settings)
    : modbusNode_(modbusNode),
      modbusSerialPort_(modbusSerialPort),
      settings_(settings)
{
}

std::unique_ptr<ApiServer> ApiServerBuilder::build()
{
    return std::make_unique<ApiServer>(
        ChangeDeviceAddressCommand(modbusNode_, modbusSerialPort_),
        settings_);
}
