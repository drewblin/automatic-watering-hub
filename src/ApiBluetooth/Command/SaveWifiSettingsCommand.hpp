#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Setting/Settings.hpp"

class SaveWifiSettingsCommand
{
public:
    explicit SaveWifiSettingsCommand(Settings &settings);
    ApiCommandResult execute(const JsonDocument &request);

private:
    Settings &settings_;
};
