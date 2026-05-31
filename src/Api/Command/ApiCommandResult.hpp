#pragma once

#include "ArduinoJson.h"

struct ApiCommandResult
{
    ApiCommandResult(uint16_t statusCode, bool success)
        : statusCode(statusCode), success(success)
    {
    }

    uint16_t statusCode;
    bool success;
    JsonDocument data;
    String error;
};
