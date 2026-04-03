#include "Hub/WaterHubBuilder.hpp"

ModbusMaster modbusNode;
WaterHub waterHub;

void setup()
{
    // todo check pins
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    WaterHubBuilder waterHubBuilder = WaterHubBuilder(
        modbusNode,
        Serial2);

    waterHub = waterHubBuilder.build(Settings());
}

void loop()
{
}
