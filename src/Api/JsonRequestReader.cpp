#include "JsonRequestReader.hpp"

#include <cerrno>
#include <cstdlib>

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

    if (!source.is<const char *>())
    {
        return false;
    }

    const char *raw = source.as<const char *>();
    char *end = nullptr;
    errno = 0;
    unsigned long parsed = strtoul(raw, &end, 0);
    if (errno != 0 || end == raw || *end != '\0' || parsed > 0xFFFF)
    {
        return false;
    }

    value = static_cast<uint16_t>(parsed);
    return true;
}
