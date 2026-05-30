#pragma once

#include "Hub/WaterHub.hpp"

class OpenValveForTimeCommand
{
public:
    explicit OpenValveForTimeCommand(WaterHub &waterHub);

    bool execute(uint8_t pin, uint32_t seconds);

private:
    WaterHub &waterHub_;
};
