#pragma once

#include "ArduinoJson.h"

class JsonRequestReader
{
public:
    static bool readRequiredUint8(JsonVariantConst json, const char *field, uint8_t &value, String &error);
    static bool readRequiredUint16(JsonVariantConst json, const char *field, uint16_t &value, String &error);

private:
    static bool parseUint8(JsonVariantConst source, uint8_t &value);
    static bool parseUint16(JsonVariantConst source, uint16_t &value);
};
