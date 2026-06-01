#include "GetWifiIpAddressCommand.hpp"

#include "WiFi.h"

ApiCommandResult GetWifiIpAddressCommand::execute()
{
    ApiCommandResult result(200, true);
    result.data["ipAddress"] = WiFi.localIP().toString();
    return result;
}
