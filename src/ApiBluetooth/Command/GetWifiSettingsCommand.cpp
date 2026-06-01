#include "GetWifiSettingsCommand.hpp"

GetWifiSettingsCommand::GetWifiSettingsCommand(const SettingsSnapshot &settings)
    : settings_(settings)
{
}

ApiCommandResult GetWifiSettingsCommand::execute()
{
    ApiCommandResult result(200, true);
    result.data["wifiSettings"]["ssid"] = settings_.wifiSettings.ssid;
    result.data["wifiSettings"]["password"] = settings_.wifiSettings.password;
    return result;
}
