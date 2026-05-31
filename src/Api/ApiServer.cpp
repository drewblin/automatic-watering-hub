#include "ApiServer.hpp"

#include "ArduinoJson.h"
#include "JsonRequestReader.hpp"

ApiServer::ApiServer(
    ChangeDeviceAddressCommand changeDeviceAddressCommand,
    OpenValveForTimeCommand openValveForTimeCommand,
    Settings &settings,
    uint16_t port)
    : changeDeviceAddressCommand_(changeDeviceAddressCommand),
      openValveForTimeCommand_(openValveForTimeCommand),
      settings_(settings),
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
    server_.on("/api/valves/open-for-time", HTTP_POST, [this]()
               { handleOpenValveForTime(); });

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
    "registerAddress": 0 (48 for soil sensor, 0 for pressure sensor),
    "saveRegisterAddress": 15, (null for soil sensor, 15 for pressure sensor)
    "saveValue": 0 (null for soil sensor, 0 for pressure sensor)
}'
    */

    JsonDocument request;
    if (!readJsonRequest(request))
    {
        return;
    }

    String error;
    uint8_t currentAddress = 0;
    uint8_t newAddress = 0;
    uint16_t registerAddress = 0;
    uint16_t saveRegisterAddress = 0;
    uint16_t saveValue = 0;
    bool save = false;
    bool hasSaveRegisterAddress = false;
    bool hasSaveValue = false;
    JsonVariantConst json = request.as<JsonVariantConst>();

    if (!JsonRequestReader::readRequiredUint8(json, "currentAddress", currentAddress, error) ||
        !JsonRequestReader::readRequiredUint8(json, "newAddress", newAddress, error) ||
        !JsonRequestReader::readRequiredUint16(json, "registerAddress", registerAddress, error) ||
        !JsonRequestReader::readOptionalUint16(json, "saveRegisterAddress", saveRegisterAddress, hasSaveRegisterAddress, error) ||
        !JsonRequestReader::readOptionalUint16(json, "saveValue", saveValue, hasSaveValue, error))
    {
        sendError(400, error);
        return;
    }

    if (hasSaveRegisterAddress != hasSaveValue)
    {
        sendError(400, "saveRegisterAddress and saveValue must be provided together");
        return;
    }

    save = hasSaveRegisterAddress && hasSaveValue;

    uint8_t status = changeDeviceAddressCommand_.execute(
        currentAddress,
        newAddress,
        registerAddress,
        save,
        saveRegisterAddress,
        saveValue);

    JsonDocument response;
    response["status"] = status;
    response["currentAddress"] = currentAddress;
    response["newAddress"] = newAddress;
    response["registerAddress"] = registerAddress;
    response["save"] = save;
    if (save)
    {
        response["saveRegisterAddress"] = saveRegisterAddress;
        response["saveValue"] = saveValue;
    }

    if (status != ChangeDeviceAddressCommand::SuccessStatus)
    {
        sendResponse(502, false, &response, "Failed to change device address");
        return;
    }

    sendSuccess(200, response);
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

    JsonDocument request;
    if (!readJsonRequest(request))
    {
        return;
    }

    String error;
    uint8_t pin = 0;
    uint32_t seconds = 0;
    JsonVariantConst json = request.as<JsonVariantConst>();

    if (!JsonRequestReader::readRequiredUint8(json, "pin", pin, error) ||
        !JsonRequestReader::readRequiredUint32(json, "seconds", seconds, error))
    {
        sendError(400, error);
        return;
    }

    if (seconds > settings_.getGlobalSettings().maximumManualValveOpenTimeSeconds ||
        seconds > INT32_MAX / 1000)
    {
        sendError(400, "Valve open time exceeds configured limit");
        return;
    }

    if (!openValveForTimeCommand_.execute(pin, seconds))
    {
        sendError(404, "Valve not found");
        return;
    }

    JsonDocument response;
    response["pin"] = pin;
    response["seconds"] = seconds;

    sendSuccess(200, response);
}

bool ApiServer::readJsonRequest(JsonDocument &request)
{
    if (!server_.hasArg("plain"))
    {
        sendError(400, "Missing JSON body");
        return false;
    }

    DeserializationError parseError = deserializeJson(request, server_.arg("plain"));
    if (parseError)
    {
        sendError(400, String("Invalid JSON: ") + parseError.c_str());
        return false;
    }

    return true;
}

void ApiServer::sendResponse(uint16_t statusCode, bool success, const JsonDocument *responseData, const String &error)
{
    JsonDocument response;
    response["success"] = success;
    if (responseData != nullptr)
    {
        response["data"] = responseData->as<JsonVariantConst>();
    }
    else
    {
        response["data"] = nullptr;
    }

    if (error.length() > 0)
    {
        response["error"] = error;
    }
    else
    {
        response["error"] = nullptr;
    }

    String payload;
    serializeJson(response, payload);
    server_.send(statusCode, "application/json", payload);
}

void ApiServer::sendSuccess(uint16_t statusCode, const JsonDocument &responseData)
{
    sendResponse(statusCode, true, &responseData, "");
}

void ApiServer::sendError(uint16_t statusCode, const String &message)
{
    sendResponse(statusCode, false, nullptr, message);
}
