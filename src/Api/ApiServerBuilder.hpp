#pragma once

#include <memory>
#include "ApiServer.hpp"
#include "Clock/Clock.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "ModbusMaster.h"
#include "Setting/Settings.hpp"

class ApiServerBuilder
{
public:
    ApiServerBuilder(
        ModbusMaster &modbusNode,
        HardwareSerial &modbusSerialPort,
        const SettingsSnapshot &settingsSnapshot,
        Settings &settings,
        Clock &clock);

    std::unique_ptr<ApiServer> build();
    void enableWaterHubRoutes(
        ApiServer &apiServer,
        WaterHub &waterHub);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
    SettingsSnapshot settingsSnapShot_;
    Settings &settings_;
    Clock &clock_;
};
