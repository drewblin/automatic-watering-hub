#include "Api/ApiServerBuilder.hpp"
#include "Clock/Clock.hpp"
#include "Connectivity/WifiConnection.hpp"
#include "Hub/WaterHubBuilder.hpp"

WifiConnection wifiConnection;
Clock systemClock;
ModbusMaster modbusNode;

WaterHubBuilder waterHubBuilder(modbusNode, Serial2);
Settings settings;
WaterHub waterHub = waterHubBuilder.build(settings);

ApiServerBuilder apiServerBuilder(modbusNode, Serial2, waterHub, settings);
ApiServer apiServer = apiServerBuilder.build();

void setup()
{
    Serial.begin(115200);

    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    waterHub.begin();
    wifiConnection.begin();
    systemClock.begin();
    apiServer.begin();
}

void loop()
{
    apiServer.handleClient();
    systemClock.loop();
    waterHub.loop();
}
