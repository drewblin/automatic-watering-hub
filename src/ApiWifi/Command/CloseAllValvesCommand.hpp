#pragma once

#include "Api/ApiCommandResult.hpp"
#include "Hub/WaterHub.hpp"

class CloseAllValvesCommand
{
public:
    explicit CloseAllValvesCommand(WaterHub &waterHub);

    ApiCommandResult execute();

private:
    WaterHub &waterHub_;
};
