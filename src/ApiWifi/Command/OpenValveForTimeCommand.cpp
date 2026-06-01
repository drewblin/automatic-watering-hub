#include "OpenValveForTimeCommand.hpp"

#include "Api/JsonRequestReader.hpp"

OpenValveForTimeCommand::OpenValveForTimeCommand(WaterHub &waterHub, const SettingsSnapshot &settings)
    : waterHub_(waterHub), settings_(settings)
{
}

ApiCommandResult OpenValveForTimeCommand::execute(const JsonDocument &request)
{
    ApiCommandResult result(400, false);
    uint8_t pin = 0;
    uint32_t seconds = 0;
    JsonVariantConst json = request.as<JsonVariantConst>();
    if (!JsonRequestReader::readRequiredUint8(json, "pin", pin, result.error) ||
        !JsonRequestReader::readRequiredUint32(json, "seconds", seconds, result.error))
    {
        return result;
    }
    if (seconds > settings_.globalSettings.maximumManualValveOpenTimeSeconds ||
        seconds > INT32_MAX / 1000)
    {
        result.error = "Valve open time exceeds configured limit";
        return result;
    }
    if (!execute(pin, seconds))
    {
        result.statusCode = 404;
        result.error = "Valve not found";
        return result;
    }

    result.statusCode = 200;
    result.success = true;
    result.data["pin"] = pin;
    result.data["seconds"] = seconds;
    return result;
}

bool OpenValveForTimeCommand::execute(uint8_t pin, uint32_t seconds)
{
    for (const auto &valve : waterHub_.getValves())
    {
        if (valve->getPin() != pin)
        {
            continue;
        }

        valve->openForTime(seconds);
        return true;
    }

    return false;
}
