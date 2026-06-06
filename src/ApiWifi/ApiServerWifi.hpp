#pragma once

#include <memory>
#include <string>
#include "ArduinoJson.h"
#include "Command/ChangeDeviceAddressCommand.hpp"
#include "Command/GetSensorMetricsCommand.hpp"
#include "Command/GetSettingsCommand.hpp"
#include "Command/OpenValveForTimeCommand.hpp"
#include "Command/SaveSettingsCommand.hpp"
#include "esp_http_server.h"

class ApiServerWifi
{
public:
    ApiServerWifi(
        ChangeDeviceAddressCommand changeDeviceAddressCommand,
        GetSettingsCommand getSettingsCommand,
        SaveSettingsCommand saveSettingsCommand,
        const std::string &apiAccessToken,
        uint16_t port = 443);

    void registerWaterHubRoutes(
        std::unique_ptr<OpenValveForTimeCommand> openValveForTimeCommand,
        std::unique_ptr<GetSensorMetricsCommand> getSensorMetricsCommand
    );

    void begin();

private:
    static constexpr size_t MAX_REQUEST_BODY_SIZE = 16 * 1024;
    static constexpr char AUTHORIZATION_HEADER[] = "Authorization";
    static constexpr char BEARER_PREFIX[] = "Bearer ";

    template <typename Command>
    ApiCommandResult execute(Command &command, httpd_req_t &request)
    {
        String payload;
        ApiCommandResult readResult = readRequestBody(request, payload);
        if (!readResult.success)
        {
            return readResult;
        }

        JsonDocument json;
        DeserializationError parseError = deserializeJson(json, payload);
        if (parseError)
        {
            ApiCommandResult result(400, false);
            result.error = String("Invalid JSON: ") + parseError.c_str();
            return result;
        }

        return command.execute(json);
    }

    template <typename Handler>
    esp_err_t authorizeAndHandle(httpd_req_t &request, Handler handler)
    {
        if (!authorize(request))
        {
            return sendCommandResult(request, ApiCommandResult(401, false));
        }
        return handler();
    }

    static esp_err_t handleChangeDeviceAddress(httpd_req_t *request);
    static esp_err_t handleOpenValveForTime(httpd_req_t *request);
    static esp_err_t handleGetSensorMetrics(httpd_req_t *request);
    static esp_err_t handleGetSettings(httpd_req_t *request);
    static esp_err_t handleSaveSettings(httpd_req_t *request);

    bool authorize(httpd_req_t &request) const;
    ApiCommandResult readRequestBody(httpd_req_t &request, String &payload) const;
    esp_err_t sendCommandResult(httpd_req_t &request, const ApiCommandResult &result) const;

    ChangeDeviceAddressCommand changeDeviceAddressCommand_;
    GetSettingsCommand getSettingsCommand_;
    SaveSettingsCommand saveSettingsCommand_;
    std::unique_ptr<OpenValveForTimeCommand> openValveForTimeCommand_;
    std::unique_ptr<GetSensorMetricsCommand> getSensorMetricsCommand_;
    std::string apiAccessToken_;
    httpd_handle_t server_ = nullptr;
    uint16_t port_;
};
