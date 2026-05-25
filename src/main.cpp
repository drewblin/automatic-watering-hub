#include "Api/ApiServerBuilder.hpp"
#include "Connectivity/WifiConnection.hpp"
#include "Hub/WaterHubBuilder.hpp"

WifiConnection wifiConnection;
ModbusMaster modbusNode;

WaterHubBuilder waterHubBuilder(modbusNode, Serial2);
WaterHub waterHub = waterHubBuilder.build(Settings());

ApiServerBuilder apiServerBuilder(modbusNode, Serial2);
ApiServer apiServer = apiServerBuilder.build();

void setup()
{
    Serial.begin(115200);

    // todo check pins
    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    wifiConnection.begin();
    apiServer.begin();
}

void loop()
{
    apiServer.handleClient();
    waterHub.loop();
}
