#include <memory>
#include "AutomaticWatering/AutomaticWatering.hpp"
#include "ApiBluetooth/ApiServerBluetoothBuilder.hpp"
#include "ApiWifi/ApiServerWifiBuilder.hpp"
#include "Clock/Clock.hpp"
#include "Connectivity/WifiConnection.hpp"
#include "Hub/WaterHubBuilder.hpp"
#include "Hypervisor/Hypervisor.hpp"
#include "Setting/Settings.hpp"

Clock systemClock;
ModbusMaster modbusNode;

WaterHubBuilder waterHubBuilder(modbusNode, Serial2);
Settings settings;
std::unique_ptr<WifiConnection> wifiConnection;
std::unique_ptr<WaterHub> waterHub;
std::unique_ptr<Hypervisor> hypervisor;
std::unique_ptr<AutomaticWatering> automaticWatering;
std::unique_ptr<ApiServerBluetooth> apiServerBluetooth;
std::unique_ptr<ApiServerWifi> apiServerWifi;

void setup()
{
    Serial.begin(115200);

    Serial2.begin(9600, SERIAL_8N1, 16, 17);

    settings.begin();
    SettingsSnapshot settingsSnapshot = settings.get();

    wifiConnection = std::make_unique<WifiConnection>(settingsSnapshot.wifiSettings);
    wifiConnection->begin();

    systemClock.begin();

    ApiServerBluetoothBuilder apiServerBluetoothBuilder(settingsSnapshot, settings);
    apiServerBluetooth = apiServerBluetoothBuilder.build();

    ApiServerWifiBuilder apiServerWifiBuilder(
        modbusNode,
        Serial2,
        settingsSnapshot,
        settings,
        systemClock);
    apiServerWifi = apiServerWifiBuilder.build();

    if (settingsSnapshot.hasRequiredWaterHubSettings())
    {
        waterHub = std::make_unique<WaterHub>(waterHubBuilder.build(settingsSnapshot));
        hypervisor = std::make_unique<Hypervisor>(*waterHub);
        automaticWatering = std::make_unique<AutomaticWatering>(*waterHub);

        apiServerWifiBuilder.enableWaterHubRoutes(*apiServerWifi, *waterHub);

        hypervisor->begin();
    }
    else
    {
        ESP_LOGE("Main", "Water hub is disabled because required settings are missing");
    }

    apiServerBluetooth->begin();
    apiServerWifi->begin();
}

void loop()
{
    systemClock.loop();
    apiServerBluetooth->loop();

    if (waterHub == nullptr)
    {
        return;
    }

    waterHub->loop();
    automaticWatering->loop();
    hypervisor->loop();
}
