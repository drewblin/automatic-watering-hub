#pragma once

#include "ApiCommandResult.hpp"
#include "Setting/Settings.hpp"

class SaveSettingsCommand
{
public:
    explicit SaveSettingsCommand(Settings &settings);
    ApiCommandResult execute(const JsonDocument &request);

private:
    static bool parseSnapshot(JsonVariantConst json, SettingsSnapshot &snapshot, String &error);

    Settings &settings_;
};
