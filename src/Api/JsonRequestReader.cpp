#include "JsonRequestReader.hpp"

#include <cmath>

bool JsonRequestReader::readRequiredObject(JsonVariantConst json, const char *field, JsonObjectConst &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!json[field].is<JsonObjectConst>())
    {
        error = String("Invalid object field: ") + field;
        return false;
    }

    value = json[field].as<JsonObjectConst>();
    return true;
}

bool JsonRequestReader::readRequiredArray(JsonVariantConst json, const char *field, JsonArrayConst &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!json[field].is<JsonArrayConst>())
    {
        error = String("Invalid array field: ") + field;
        return false;
    }

    value = json[field].as<JsonArrayConst>();
    return true;
}

bool JsonRequestReader::readRequiredString(JsonVariantConst json, const char *field, std::string &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!json[field].is<const char *>())
    {
        error = String("Invalid string field: ") + field;
        return false;
    }

    value = json[field].as<const char *>();
    return true;
}

bool JsonRequestReader::readRequiredUint8(JsonVariantConst json, const char *field, uint8_t &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!parseUint8(json[field], value))
    {
        error = String("Invalid uint8 field: ") + field;
        return false;
    }

    return true;
}

bool JsonRequestReader::readRequiredUint16(JsonVariantConst json, const char *field, uint16_t &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!parseUint16(json[field], value))
    {
        error = String("Invalid uint16 field: ") + field;
        return false;
    }

    return true;
}

bool JsonRequestReader::readRequiredUint32(JsonVariantConst json, const char *field, uint32_t &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!parseUint32(json[field], value))
    {
        error = String("Invalid uint32 field: ") + field;
        return false;
    }

    return true;
}

bool JsonRequestReader::readRequiredPositiveFloat(JsonVariantConst json, const char *field, float &value, String &error)
{
    if (json[field].isNull())
    {
        error = String("Missing field: ") + field;
        return false;
    }

    if (!parsePositiveFloat(json[field], value))
    {
        error = String("Invalid positive float field: ") + field;
        return false;
    }

    return true;
}

bool JsonRequestReader::readOptionalUint16(JsonVariantConst json, const char *field, uint16_t &value, bool &present, String &error)
{
    if (json[field].isNull())
    {
        present = false;
        return true;
    }

    if (!parseUint16(json[field], value))
    {
        error = String("Invalid uint16 field: ") + field;
        return false;
    }

    present = true;
    return true;
}

bool JsonRequestReader::parseUint8(JsonVariantConst source, uint8_t &value)
{
    uint16_t parsed = 0;
    if (!parseUint16(source, parsed) || parsed > 0xFF)
    {
        return false;
    }

    value = static_cast<uint8_t>(parsed);
    return true;
}

bool JsonRequestReader::parseUint16(JsonVariantConst source, uint16_t &value)
{
    if (source.is<int>())
    {
        long parsed = source.as<long>();
        if (parsed < 0 || parsed > 0xFFFF)
        {
            return false;
        }

        value = static_cast<uint16_t>(parsed);
        return true;
    }

    return false;
}

bool JsonRequestReader::parseUint32(JsonVariantConst source, uint32_t &value)
{
    if (source.is<uint32_t>())
    {
        value = source.as<uint32_t>();
        return true;
    }

    return false;
}

bool JsonRequestReader::parsePositiveFloat(JsonVariantConst source, float &value)
{
    if (!source.is<float>())
    {
        return false;
    }

    float parsed = source.as<float>();
    if (!std::isfinite(parsed) || parsed <= 0)
    {
        return false;
    }

    value = parsed;
    return true;
}
