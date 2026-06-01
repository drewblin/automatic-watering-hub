#pragma once

#include "ArduinoJson.h"

struct ApiCommandResult
{
    ApiCommandResult(uint16_t statusCode, bool success)
        : statusCode(statusCode), success(success)
    {
    }

    const char *getHttpStatus() const
    {
        switch (statusCode)
        {
        case 200:
            return "200 OK";
        case 400:
            return "400 Bad Request";
        case 401:
            return "401 Unauthorized";
        case 404:
            return "404 Not Found";
        case 413:
            return "413 Content Too Large";
        case 500:
            return "500 Internal Server Error";
        case 503:
            return "503 Service Unavailable";
        default:
            return "500 Internal Server Error";
        }
    }

    uint16_t statusCode;
    bool success;
    JsonDocument data;
    String error;
};
