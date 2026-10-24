#include "ApiServerWifiBuilder.hpp"

#include <memory>

#include "Command/GetSensorMetricsCommand.hpp"
#include "Command/CloseAllValvesCommand.hpp"
#include "Command/GetSettingsCommand.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"
#include "Command/ReadModbusRegisterCommand.hpp"
#include "Command/WriteModbusRegisterCommand.hpp"

ApiServerWifiBuilder::ApiServerWifiBuilder(
    ModbusMaster &modbusNode,
    HardwareSerial &modbusSerialPort,
    ModbusSensorPower &modbusSensorPower,
    const SettingsSnapshot &settingsSnapshot,
    Settings &settings,
    Clock &clock)
    : modbusNode_(modbusNode),
      modbusSerialPort_(modbusSerialPort),
      modbusSensorPower_(modbusSensorPower),
      settingsSnapShot_(settingsSnapshot),
      settings_(settings),
      clock_(clock)
{
}

std::unique_ptr<ApiServerWifi> ApiServerWifiBuilder::build()
{
    return std::make_unique<ApiServerWifi>(
        WriteModbusRegisterCommand(modbusNode_, modbusSerialPort_, modbusSensorPower_),
        ReadModbusRegisterCommand(modbusNode_, modbusSerialPort_, modbusSensorPower_),
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
        std::make_unique<CloseAllValvesCommand>(waterHub),
        std::make_unique<GetSensorMetricsCommand>(waterHub, settingsSnapShot_));
}
