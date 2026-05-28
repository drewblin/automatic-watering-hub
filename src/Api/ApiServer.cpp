#include "ApiServer.hpp"

#include "ArduinoJson.h"
#include "JsonRequestReader.hpp"

ApiServer::ApiServer(ChangeDeviceAddressCommand changeDeviceAddressCommand, uint16_t port)
    : changeDeviceAddressCommand_(changeDeviceAddressCommand),
      server_(port)
{
}

void ApiServer::begin()
{
    registerRoutes();
    server_.begin();
}

void ApiServer::handleClient()
{
    server_.handleClient();
}

void ApiServer::registerRoutes()
{
    server_.on("/api/modbus/device-address", HTTP_POST, [this]()
               { handleChangeDeviceAddress(); });

    server_.onNotFound([this]()
                       { sendError(404, "Not found"); });
}

void ApiServer::handleChangeDeviceAddress()
{
    /*
curl -X POST http://192.168.0.104/api/modbus/device-address \
-H "Content-Type: application/json" \
-d '{
    "currentAddress": 1,
    "newAddress": 2,
    "registerAddress": 0
}'
    */

    if (!server_.hasArg("plain"))
    {
        sendError(400, "Missing JSON body");
        return;
    }

    JsonDocument request;
    DeserializationError parseError = deserializeJson(request, server_.arg("plain"));
    if (parseError)
    {
        sendError(400, String("Invalid JSON: ") + parseError.c_str());
        return;
    }

    String error;
    uint8_t currentAddress = 0;
    uint8_t newAddress = 0;
    uint16_t registerAddress = 0;
    JsonVariantConst json = request.as<JsonVariantConst>();

    if (!JsonRequestReader::readRequiredUint8(json, "currentAddress", currentAddress, error) ||
        !JsonRequestReader::readRequiredUint8(json, "newAddress", newAddress, error) ||
        !JsonRequestReader::readRequiredUint16(json, "registerAddress", registerAddress, error))
    {
        sendError(400, error);
        return;
    }

    uint8_t status = changeDeviceAddressCommand_.execute(currentAddress, newAddress, registerAddress);

    JsonDocument response;
    response["success"] = status == ChangeDeviceAddressCommand::SuccessStatus;
    response["status"] = status;
    response["currentAddress"] = currentAddress;
    response["newAddress"] = newAddress;
    response["registerAddress"] = registerAddress;

    String payload;
    serializeJson(response, payload);
    sendJson(status == ChangeDeviceAddressCommand::SuccessStatus ? 200 : 502, payload);
}

void ApiServer::sendJson(uint16_t statusCode, const String &payload)
{
    server_.send(statusCode, "application/json", payload);
}

void ApiServer::sendError(uint16_t statusCode, const String &message)
{
    JsonDocument response;
    response["success"] = false;
    response["error"] = message;

    String payload;
    serializeJson(response, payload);
    sendJson(statusCode, payload);
}
