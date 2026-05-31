#include "OpenValveForTimeCommand.hpp"

OpenValveForTimeCommand::OpenValveForTimeCommand(WaterHub &waterHub)
    : waterHub_(waterHub)
{
}

bool OpenValveForTimeCommand::execute(uint8_t pin, uint32_t seconds)
{
    for (const auto &valve : waterHub_.getValves())
    {
        if (valve->getPin() != pin)
        {
            continue;
        }

        valve->openForTime(seconds);
        return true;
    }

    return false;
}
