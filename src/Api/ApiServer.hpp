#pragma once

#include "ArduinoJson.h"
#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"
#include "Setting/Settings.hpp"
#include "WebServer.h"

class ApiServer
{
public:
    ApiServer(
        ChangeDeviceAddressCommand changeDeviceAddressCommand,
        OpenValveForTimeCommand openValveForTimeCommand,
        Settings &settings,
        uint16_t port = 80);

    void begin();
    void handleClient();

private:
    void registerRoutes();
    void handleChangeDeviceAddress();
    void handleOpenValveForTime();
    bool readJsonRequest(JsonDocument &request);
    void sendResponse(uint16_t statusCode, bool success, const JsonDocument *data, const String &error);
    void sendSuccess(uint16_t statusCode, const JsonDocument &data);
    void sendError(uint16_t statusCode, const String &message);

    ChangeDeviceAddressCommand changeDeviceAddressCommand_;
    OpenValveForTimeCommand openValveForTimeCommand_;
    Settings &settings_;
    WebServer server_;
};
