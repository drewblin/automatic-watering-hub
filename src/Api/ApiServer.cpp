#include "ApiServer.hpp"

#include <Arduino.h>

ApiServer::ApiServer(
    ChangeDeviceAddressCommand changeDeviceAddressCommand,
    GetSettingsCommand getSettingsCommand,
    SaveSettingsCommand saveSettingsCommand,
    uint16_t port)
    : changeDeviceAddressCommand_(changeDeviceAddressCommand),
      getSettingsCommand_(getSettingsCommand),
      saveSettingsCommand_(saveSettingsCommand),
      server_(port)
{
    server_.on("/api/modbus/device-address", HTTP_POST, [this]()
               { handleChangeDeviceAddress(); });
    server_.on("/api/settings", HTTP_GET, [this]()
               { handleGetSettings(); });
    server_.on("/api/settings", HTTP_PUT, [this]()
               { handleSaveSettings(); });

    server_.onNotFound([this]()
                       {
                           ApiCommandResult result(404, false);
                           result.error = "Not found";
                           sendCommandResult(result);
                       });
}

void ApiServer::registerWaterHubRoutes(
    std::unique_ptr<OpenValveForTimeCommand> openValveForTimeCommand)
{
    openValveForTimeCommand_ = std::move(openValveForTimeCommand);

    server_.on("/api/valves/open-for-time", HTTP_POST, [this]()
               { handleOpenValveForTime(); });
}

void ApiServer::begin()
{
    server_.begin();
}

void ApiServer::handleClient()
{
    server_.handleClient();
}

void ApiServer::handleChangeDeviceAddress()
{
    /*
curl -X POST http://192.168.0.104/api/modbus/device-address \
-H "Content-Type: application/json" \
-d '{
    "currentAddress": 1,
    "newAddress": 2,
    "registerAddress": 0 (48 for soil sensor, 0 for pressure sensor),
    "saveRegisterAddress": 15, (null for soil sensor, 15 for pressure sensor)
    "saveValue": 0 (null for soil sensor, 0 for pressure sensor)
}'
    */

    sendCommandResult(execute(changeDeviceAddressCommand_));
}

void ApiServer::handleOpenValveForTime()
{
    /*
curl -X POST http://192.168.0.104/api/valves/open-for-time \
-H "Content-Type: application/json" \
-d '{
    "pin": 22,
    "seconds": 10
}'
    */

    if (openValveForTimeCommand_ == nullptr)
    {
        ApiCommandResult result(503, false);
        result.error = "Water hub is not available";
        sendCommandResult(result);
        return;
    }

    sendCommandResult(execute(*openValveForTimeCommand_));
}

void ApiServer::handleGetSettings()
{
    sendCommandResult(getSettingsCommand_.execute());
}

void ApiServer::handleSaveSettings()
{
    ApiCommandResult result = execute(saveSettingsCommand_);
    sendCommandResult(result);
    if (!result.success)
    {
        return;
    }

    delay(100);
    ESP.restart();
}

void ApiServer::sendCommandResult(const ApiCommandResult &result)
{
    JsonDocument response;
    response["success"] = result.success;
    response["data"] = result.data.as<JsonVariantConst>();

    if (result.error.length() > 0)
    {
        response["error"] = result.error;
    }
    else
    {
        response["error"] = nullptr;
    }

    String payload;
    serializeJson(response, payload);
    server_.send(result.statusCode, "application/json", payload);
}
