#include "WifiConnection.hpp"
#include "WiFi.h"

// todo use bluetooth for wifi config
#define WIFI_SSID "Lypky"
#define WIFI_PASSWORD "79348454"

void WifiConnection::begin()
{
    Serial.print("Starting WiFi: ");

    WiFi.mode(WIFI_STA);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

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
