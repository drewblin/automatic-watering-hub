#include "CloseAllValvesCommand.hpp"

CloseAllValvesCommand::CloseAllValvesCommand(WaterHub &waterHub)
    : waterHub_(waterHub)
{
}

ApiCommandResult CloseAllValvesCommand::execute()
{
    waterHub_.closeAllValves();
    ApiCommandResult result(200, true);
    result.data.to<JsonObject>();
    return result;
}
