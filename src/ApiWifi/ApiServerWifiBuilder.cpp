#include "ApiServerWifiBuilder.hpp"

#include <memory>

#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/GetSensorMetricsCommand.hpp"
#include "Command/GetSettingsCommand.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"

ApiServerWifiBuilder::ApiServerWifiBuilder(
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

std::unique_ptr<ApiServerWifi> ApiServerWifiBuilder::build()
{
    return std::make_unique<ApiServerWifi>(
        ChangeDeviceAddressCommand(modbusNode_, modbusSerialPort_),
        GetSettingsCommand(settingsSnapShot_, clock_),
        SaveSettingsCommand(settings_),
        settingsSnapShot_.apiAccessToken);
}

void ApiServerWifiBuilder::enableWaterHubRoutes(
    ApiServerWifi &apiServerWifi,
    WaterHub &waterHub)
{
    apiServerWifi.registerWaterHubRoutes(
        std::make_unique<OpenValveForTimeCommand>(waterHub, settingsSnapShot_),
        std::make_unique<GetSensorMetricsCommand>(waterHub, settingsSnapShot_));
}
