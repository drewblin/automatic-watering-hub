#include "ApiServerWifi.hpp"

#include <Arduino.h>
#include "TlsCertificate.hpp"
#include "esp_https_server.h"

ApiServerWifi::ApiServerWifi(
    ChangeDeviceAddressCommand changeDeviceAddressCommand,
    GetSettingsCommand getSettingsCommand,
    SaveSettingsCommand saveSettingsCommand,
    const std::string &apiAccessToken,
    uint16_t port)
    : changeDeviceAddressCommand_(changeDeviceAddressCommand),
      getSettingsCommand_(getSettingsCommand),
      saveSettingsCommand_(saveSettingsCommand),
      apiAccessToken_(apiAccessToken),
      port_(port)
{
}

void ApiServerWifi::registerWaterHubRoutes(
    std::unique_ptr<OpenValveForTimeCommand> openValveForTimeCommand)
{
    openValveForTimeCommand_ = std::move(openValveForTimeCommand);
}

void ApiServerWifi::begin()
{
    httpd_ssl_config_t config = HTTPD_SSL_CONFIG_DEFAULT();
    config.port_secure = port_;
    config.servercert = TlsCertificate::Certificate;
    config.servercert_len = sizeof(TlsCertificate::Certificate);
    config.prvtkey_pem = TlsCertificate::PrivateKey;
    config.prvtkey_len = sizeof(TlsCertificate::PrivateKey);

    if (httpd_ssl_start(&server_, &config) != ESP_OK)
    {
        ESP_LOGE("ApiServerWifi", "Failed to start HTTPS server");
        return;
    }

    httpd_uri_t changeDeviceAddress = {
        .uri = "/api/modbus/device-address",
        .method = HTTP_POST,
        .handler = handleChangeDeviceAddress,
        .user_ctx = this};
    httpd_register_uri_handler(server_, &changeDeviceAddress);

    httpd_uri_t getSettings = {
        .uri = "/api/settings",
        .method = HTTP_GET,
        .handler = handleGetSettings,
        .user_ctx = this};
    httpd_register_uri_handler(server_, &getSettings);

    httpd_uri_t saveSettings = {
        .uri = "/api/settings",
        .method = HTTP_PUT,
        .handler = handleSaveSettings,
        .user_ctx = this};
    httpd_register_uri_handler(server_, &saveSettings);

    if (openValveForTimeCommand_ != nullptr)
    {
        httpd_uri_t openValveForTime = {
            .uri = "/api/valves/open-for-time",
            .method = HTTP_POST,
            .handler = handleOpenValveForTime,
            .user_ctx = this};
        httpd_register_uri_handler(server_, &openValveForTime);
    }
}

bool ApiServerWifi::authorize(httpd_req_t &request) const
{
    size_t headerLength = httpd_req_get_hdr_value_len(&request, AUTHORIZATION_HEADER);
    if (headerLength == 0 || headerLength > apiAccessToken_.size() + strlen(BEARER_PREFIX))
    {
        return false;
    }

    std::string header(headerLength + 1, '\0');
    if (httpd_req_get_hdr_value_str(&request, AUTHORIZATION_HEADER, header.data(), header.size()) != ESP_OK)
    {
        return false;
    }
    header.resize(headerLength);
    return header == std::string(BEARER_PREFIX) + apiAccessToken_;
}

ApiCommandResult ApiServerWifi::readRequestBody(httpd_req_t &request, String &payload) const
{
    if (request.content_len > MAX_REQUEST_BODY_SIZE)
    {
        ApiCommandResult result(413, false);
        result.error = "Request body is too large";
        return result;
    }

    payload.reserve(request.content_len);
    while (payload.length() < request.content_len)
    {
        char buffer[512];
        size_t bytesToRead = min(sizeof(buffer), request.content_len - payload.length());
        int received = httpd_req_recv(&request, buffer, bytesToRead);
        if (received <= 0)
        {
            ApiCommandResult result(400, false);
            result.error = "Failed to read request body";
            return result;
        }
        payload.concat(buffer, received);
    }
    return ApiCommandResult(200, true);
}

esp_err_t ApiServerWifi::handleChangeDeviceAddress(httpd_req_t *request)
{
    ApiServerWifi *server = static_cast<ApiServerWifi *>(request->user_ctx);
    return server->authorizeAndHandle(*request, [&]()
    {
        return server->sendCommandResult(*request, server->execute(server->changeDeviceAddressCommand_, *request));
    });
}

esp_err_t ApiServerWifi::handleOpenValveForTime(httpd_req_t *request)
{
    ApiServerWifi *server = static_cast<ApiServerWifi *>(request->user_ctx);
    return server->authorizeAndHandle(*request, [&]()
    {
        if (server->openValveForTimeCommand_ == nullptr)
        {
            ApiCommandResult result(503, false);
            result.error = "Water hub is not available";
            return server->sendCommandResult(*request, result);
        }
        return server->sendCommandResult(*request, server->execute(*server->openValveForTimeCommand_, *request));
    });
}

esp_err_t ApiServerWifi::handleGetSettings(httpd_req_t *request)
{
    ApiServerWifi *server = static_cast<ApiServerWifi *>(request->user_ctx);
    return server->authorizeAndHandle(*request, [&]()
    {
        return server->sendCommandResult(*request, server->getSettingsCommand_.execute());
    });
}

esp_err_t ApiServerWifi::handleSaveSettings(httpd_req_t *request)
{
    ApiServerWifi *server = static_cast<ApiServerWifi *>(request->user_ctx);
    return server->authorizeAndHandle(*request, [&]()
    {
        ApiCommandResult result = server->execute(server->saveSettingsCommand_, *request);
        esp_err_t responseResult = server->sendCommandResult(*request, result);
        if (result.success)
        {
            delay(100);
            ESP.restart();
        }
        return responseResult;
    });
}

esp_err_t ApiServerWifi::sendCommandResult(httpd_req_t &request, const ApiCommandResult &result) const
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
    httpd_resp_set_status(&request, result.getHttpStatus());
    httpd_resp_set_type(&request, HTTPD_TYPE_JSON);
    return httpd_resp_send(&request, payload.c_str(), payload.length());
}
