#pragma once

#include "ArduinoJson.h"

class JsonRequestReader
{
public:
    static bool readRequiredUint8(JsonVariantConst json, const char *field, uint8_t &value, String &error);
    static bool readRequiredUint16(JsonVariantConst json, const char *field, uint16_t &value, String &error);
    static bool readRequiredUint32(JsonVariantConst json, const char *field, uint32_t &value, String &error);
    static bool readOptionalUint16(JsonVariantConst json, const char *field, uint16_t &value, bool &present, String &error);

private:
    static bool parseUint8(JsonVariantConst source, uint8_t &value);
    static bool parseUint16(JsonVariantConst source, uint16_t &value);
    static bool parseUint32(JsonVariantConst source, uint32_t &value);
};
