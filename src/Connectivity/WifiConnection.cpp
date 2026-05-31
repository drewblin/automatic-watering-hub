#include "WifiConnection.hpp"
#include "WiFi.h"

WifiConnection::WifiConnection(WifiSettings settings)
    : settings_(settings)
{
}

void WifiConnection::begin()
{
    Serial.print("Starting WiFi: ");

    WiFi.mode(WIFI_STA);
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
    }
}
