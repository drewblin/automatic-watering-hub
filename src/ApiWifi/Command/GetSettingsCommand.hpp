#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Clock/Clock.hpp"
#include "Setting/SettingsSnapshot.hpp"

class GetSettingsCommand
{
public:
    GetSettingsCommand(const SettingsSnapshot &settings, Clock &clock);
    ApiCommandResult execute();

private:
    const SettingsSnapshot settings_;
    Clock &clock_;
};
