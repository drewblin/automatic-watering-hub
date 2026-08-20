#include "GetWifiIpAddressCommand.hpp"

#include "WiFi.h"

GetWifiIpAddressCommand::GetWifiIpAddressCommand(const SettingsSnapshot &settings)
    : hostname_(settings.deviceHostname)
{
}

ApiCommandResult GetWifiIpAddressCommand::execute()
{
    ApiCommandResult result(200, true);
    result.data["ipAddress"] = WiFi.localIP().toString();
    result.data["hostname"] = hostname_;
    result.data["localHostname"] = hostname_.empty() ? "" : hostname_ + ".local";
    return result;
}
