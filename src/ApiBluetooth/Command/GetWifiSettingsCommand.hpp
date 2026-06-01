#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Setting/SettingsSnapshot.hpp"

class GetWifiSettingsCommand
{
public:
    explicit GetWifiSettingsCommand(const SettingsSnapshot &settings);
    ApiCommandResult execute();

private:
    SettingsSnapshot settings_;
};
