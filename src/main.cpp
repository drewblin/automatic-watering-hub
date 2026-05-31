#include <memory>
#include "AutomaticWatering/AutomaticWatering.hpp"
#include "Api/ApiServerBuilder.hpp"
#include "Clock/Clock.hpp"
#include "Connectivity/WifiConnection.hpp"
#include "Hub/WaterHubBuilder.hpp"
#include "Hypervisor/Hypervisor.hpp"

Clock systemClock;
ModbusMaster modbusNode;

WaterHubBuilder waterHubBuilder(modbusNode, Serial2);
Settings settings;
std::unique_ptr<WifiConnection> wifiConnection;
std::unique_ptr<WaterHub> waterHub;
std::unique_ptr<Hypervisor> hypervisor;
std::unique_ptr<AutomaticWatering> automaticWatering;
std::unique_ptr<ApiServer> apiServer;

void setup()
{
    Serial.begin(115200);

    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    settings.begin();
    wifiConnection = std::make_unique<WifiConnection>(settings.getWifiSettings());
    wifiConnection->begin();
    systemClock.begin();
    ApiServerBuilder apiServerBuilder(modbusNode, Serial2, settings);
    apiServer = apiServerBuilder.build();

    if (!settings.hasRequiredWaterHubSettings())
    {
        ESP_LOGE("Main", "Water hub is disabled because required settings are missing");
        apiServer->begin();
        return;
    }

    waterHub = std::make_unique<WaterHub>(waterHubBuilder.build(settings));
    hypervisor = std::make_unique<Hypervisor>(*waterHub);
    automaticWatering = std::make_unique<AutomaticWatering>(*waterHub);
    apiServer->enableWaterHubRoutes(*waterHub);

    hypervisor->begin();
    apiServer->begin();
}

void loop()
{
    systemClock.loop();
    apiServer->handleClient();
    if (waterHub == nullptr)
    {
        return;
    }

    waterHub->loop();
    automaticWatering->loop();
    hypervisor->loop();
}
