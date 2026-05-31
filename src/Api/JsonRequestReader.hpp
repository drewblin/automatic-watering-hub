#pragma once

#include "ArduinoJson.h"

#include <string>

class JsonRequestReader
{
public:
    static bool readRequiredObject(JsonVariantConst json, const char *field, JsonObjectConst &value, String &error);
    static bool readRequiredArray(JsonVariantConst json, const char *field, JsonArrayConst &value, String &error);
    static bool readRequiredString(JsonVariantConst json, const char *field, std::string &value, String &error);
    static bool readRequiredUint8(JsonVariantConst json, const char *field, uint8_t &value, String &error);
    static bool readRequiredUint16(JsonVariantConst json, const char *field, uint16_t &value, String &error);
    static bool readRequiredUint32(JsonVariantConst json, const char *field, uint32_t &value, String &error);
    static bool readRequiredPositiveFloat(JsonVariantConst json, const char *field, float &value, String &error);
    static bool readOptionalUint16(JsonVariantConst json, const char *field, uint16_t &value, bool &present, String &error);

private:
    static bool parseUint8(JsonVariantConst source, uint8_t &value);
    static bool parseUint16(JsonVariantConst source, uint16_t &value);
    static bool parseUint32(JsonVariantConst source, uint32_t &value);
    static bool parsePositiveFloat(JsonVariantConst source, float &value);
};
