#include "GetApiAccessTokenCommand.hpp"

GetApiAccessTokenCommand::GetApiAccessTokenCommand(const SettingsSnapshot &settings)
    : settings_(settings)
{
}

ApiCommandResult GetApiAccessTokenCommand::execute()
{
    ApiCommandResult result(200, true);
    result.data["apiAccessToken"] = settings_.apiAccessToken;
    return result;
}
