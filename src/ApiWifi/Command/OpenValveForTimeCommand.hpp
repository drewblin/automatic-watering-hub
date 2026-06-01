#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Hub/WaterHub.hpp"
#include "Setting/SettingsSnapshot.hpp"

class OpenValveForTimeCommand
{
public:
    OpenValveForTimeCommand(WaterHub &waterHub, const SettingsSnapshot &settings);

    ApiCommandResult execute(const JsonDocument &request);
    bool execute(uint8_t pin, uint32_t seconds);

private:
    WaterHub &waterHub_;
    const SettingsSnapshot settings_;
};
