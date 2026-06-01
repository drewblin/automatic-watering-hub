#pragma once

#include <memory>
#include "ArduinoJson.h"
#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/GetSettingsCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "WebServer.h"

class ApiServerWifi
{
public:
    ApiServerWifi(
        ChangeDeviceAddressCommand changeDeviceAddressCommand,
        GetSettingsCommand getSettingsCommand,
        SaveSettingsCommand saveSettingsCommand,
        uint16_t port = 80);

    void registerWaterHubRoutes(
        std::unique_ptr<OpenValveForTimeCommand> openValveForTimeCommand
    );

    void begin();
    void handleClient();

private:
    template <typename Command>
    ApiCommandResult execute(Command &command)
    {
        JsonDocument request;
        DeserializationError parseError = deserializeJson(request, server_.arg("plain"));
        if (parseError)
        {
            ApiCommandResult result(400, false);
            result.error = String("Invalid JSON: ") + parseError.c_str();
            return result;
        }

        return command.execute(request);
    }

    void handleChangeDeviceAddress();
    void handleOpenValveForTime();
    void handleGetSettings();
    void handleSaveSettings();
    void sendCommandResult(const ApiCommandResult &result);

    ChangeDeviceAddressCommand changeDeviceAddressCommand_;
    GetSettingsCommand getSettingsCommand_;
    SaveSettingsCommand saveSettingsCommand_;
    std::unique_ptr<OpenValveForTimeCommand> openValveForTimeCommand_;
    WebServer server_;
};
