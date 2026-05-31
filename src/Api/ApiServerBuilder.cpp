#include "ApiServerBuilder.hpp"

#include <memory>

#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/GetSettingsCommand.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"

ApiServerBuilder::ApiServerBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort,
    const SettingsSnapshot &settingsSnapshot,
    Settings &settings,
    Clock &clock)
    : modbusNode_(modbusNode),
      modbusSerialPort_(modbusSerialPort),
      settingsSnapShot_(settingsSnapshot),
      settings_(settings),
      clock_(clock)
{
}

std::unique_ptr<ApiServer> ApiServerBuilder::build()
{
    return std::make_unique<ApiServer>(
        ChangeDeviceAddressCommand(modbusNode_, modbusSerialPort_),
        GetSettingsCommand(settingsSnapShot_, clock_),
        SaveSettingsCommand(settings_));
}

void ApiServerBuilder::enableWaterHubRoutes(
    ApiServer &apiServer,
    WaterHub &waterHub)
{
    apiServer.registerWaterHubRoutes(
        std::make_unique<OpenValveForTimeCommand>(waterHub, settingsSnapShot_));
}
