#pragma once

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
    void sendJson(uint16_t statusCode, const String &payload);
    void sendError(uint16_t statusCode, const String &message);

    ChangeDeviceAddressCommand changeDeviceAddressCommand_;
    OpenValveForTimeCommand openValveForTimeCommand_;
    Settings &settings_;
    WebServer server_;
};
