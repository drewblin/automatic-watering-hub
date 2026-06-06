#include "WifiConnection.hpp"
#include "WiFi.h"
#include "Logging/Logger.hpp"

WifiConnection::WifiConnection(WifiSettings settings)
    : settings_(settings)
{
}

bool WifiConnection::begin()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        return true;
    }

    if (settings_.ssid.empty())
    {
        Logger::w("WifiConnection", "WiFi was not started because SSID is empty");
        return false;
    }

    Logger::i("WifiConnection", "Starting WiFi");

    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    WiFi.setSleep(WIFI_PS_MAX_MODEM);
    WiFi.begin(settings_.ssid.c_str(), settings_.password.c_str());

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000)
    {
        Serial.print(".");
        delay(250);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        Serial.print("WiFi connected. IP: ");
        Serial.println(WiFi.localIP());
        WiFi.setSleep(WIFI_PS_MAX_MODEM);
        return true;
    }

    Logger::w("WifiConnection", "Failed to connect to WiFi within timeout");
    return false;
}
