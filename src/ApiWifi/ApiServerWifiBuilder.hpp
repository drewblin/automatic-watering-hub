#pragma once

#include <memory>
#include "ApiServerWifi.hpp"
#include "Clock/Clock.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "ModbusMaster.h"
#include "Setting/Settings.hpp"

class ApiServerWifiBuilder
{
public:
    ApiServerWifiBuilder(
        ModbusMaster &modbusNode,
        HardwareSerial &modbusSerialPort,
        const SettingsSnapshot &settingsSnapshot,
        Settings &settings,
        Clock &clock);

    std::unique_ptr<ApiServerWifi> build();
    void enableWaterHubRoutes(
        ApiServerWifi &apiServerWifi,
        WaterHub &waterHub);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
    SettingsSnapshot settingsSnapShot_;
    Settings &settings_;
    Clock &clock_;
};
