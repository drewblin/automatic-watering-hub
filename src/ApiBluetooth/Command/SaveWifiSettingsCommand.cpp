#include "SaveWifiSettingsCommand.hpp"

#include "Api/JsonRequestReader.hpp"

SaveWifiSettingsCommand::SaveWifiSettingsCommand(Settings &settings)
    : settings_(settings)
{
}

ApiCommandResult SaveWifiSettingsCommand::execute(const JsonDocument &request)
{
    ApiCommandResult result(400, false);
    WifiSettings wifiSettings;
    JsonVariantConst json = request.as<JsonVariantConst>();
    if (!JsonRequestReader::readRequiredString(json, "ssid", wifiSettings.ssid, result.error) ||
        !JsonRequestReader::readRequiredString(json, "password", wifiSettings.password, result.error))
    {
        return result;
    }
    if (!settings_.saveWifiSettings(wifiSettings, result.error))
    {
        result.statusCode = 500;
        return result;
    }

    result.statusCode = 200;
    result.success = true;
    result.data["restartScheduled"] = true;
    return result;
}
