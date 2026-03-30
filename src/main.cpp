#include "Hub/WaterHubBuilder.hpp"

WaterHub waterHub;

void setup()
{
    waterHub = WaterHubBuilder().build(Settings());
}

void loop()
{
}
