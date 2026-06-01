#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Setting/SettingsSnapshot.hpp"

class GetApiAccessTokenCommand
{
public:
    explicit GetApiAccessTokenCommand(const SettingsSnapshot &settings);
    ApiCommandResult execute();

private:
    SettingsSnapshot settings_;
};
