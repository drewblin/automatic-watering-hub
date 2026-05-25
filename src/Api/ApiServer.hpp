#pragma once

#include "Command/ChangeDeviceAddressCommand.hpp"
#include "WebServer.h"

class ApiServer
{
public:
    ApiServer(ChangeDeviceAddressCommand changeDeviceAddressCommand, uint16_t port = 80);

    void begin();
    void handleClient();

private:
    void registerRoutes();
    void handleChangeDeviceAddress();
    void sendJson(uint16_t statusCode, const String &payload);
    void sendError(uint16_t statusCode, const String &message);

    ChangeDeviceAddressCommand changeDeviceAddressCommand_;
    WebServer server_;
};
