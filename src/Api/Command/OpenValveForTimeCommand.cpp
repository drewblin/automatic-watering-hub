#include "OpenValveForTimeCommand.hpp"

OpenValveForTimeCommand::OpenValveForTimeCommand(WaterHub &waterHub)
    : waterHub_(waterHub)
{
}

bool OpenValveForTimeCommand::execute(uint8_t pin, uint32_t seconds)
{
    return waterHub_.openValveForTime(pin, seconds);
}
