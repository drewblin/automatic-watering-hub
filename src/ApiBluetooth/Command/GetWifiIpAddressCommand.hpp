#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Setting/SettingsSnapshot.hpp"
#include <string>

class GetWifiIpAddressCommand
{
public:
    explicit GetWifiIpAddressCommand(const SettingsSnapshot &settings);
    ApiCommandResult execute();

private:
    std::string hostname_;
};
