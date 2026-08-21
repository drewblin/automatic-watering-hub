#include "WifiConnection.hpp"
#include <utility>
#include <ESPmDNS.h>
#include "WiFi.h"
#include "Logging/Logger.hpp"

WifiConnection::WifiConnection(WifiSettings settings, std::string hostname)
    : settings_(settings),
      hostname_(std::move(hostname))
{
}

bool WifiConnection::begin()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!mdnsServiceRegistered_)
        {
            startMdns();
        }
        return true;
    }

    stopMdns();

    if (settings_.ssid.empty())
    {
        Logger::w("WifiConnection", "WiFi was not started because SSID is empty");
        return false;
    }

    Logger::i("WifiConnection", "Starting WiFi");

    startWifi();

    uint32_t start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < WifiConnectTimeoutMs)
    {
        Serial.print(".");
        delay(250);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        wifiConnected_ = true;
        Serial.print("WiFi connected. IP: ");
        Serial.println(WiFi.localIP());
        startMdns();
        return true;
    }

    Logger::w("WifiConnection", "Failed to connect to WiFi within timeout");
    return false;
}

void WifiConnection::loop()
{
    if (WiFi.status() != WL_CONNECTED)
    {
        if (wifiConnected_)
        {
            Logger::w("WifiConnection", "WiFi connection lost");
            wifiConnected_ = false;
        }

        stopMdns();
        if (!settings_.ssid.empty() && millis() - lastWifiReconnectAttemptTimeMs_ >= WifiReconnectIntervalMs)
        {
            Logger::i("WifiConnection", "Reconnecting WiFi");
            startWifi();
        }
        return;
    }

    if (!wifiConnected_)
    {
        wifiConnected_ = true;
        Logger::i("WifiConnection", "WiFi reconnected. IP: %s", WiFi.localIP().toString().c_str());
        startMdns();
        return;
    }

    if (!mdnsServiceRegistered_ && millis() - lastMdnsStartAttemptTimeMs_ >= MdnsRetryIntervalMs)
    {
        startMdns();
    }
}

void WifiConnection::configureWifi()
{
    WiFi.persistent(false);
    WiFi.mode(WIFI_STA);
    if (!hostname_.empty())
    {
        WiFi.setHostname(hostname_.c_str());
    }
    WiFi.setSleep(WIFI_PS_MAX_MODEM);
}

void WifiConnection::startWifi()
{
    configureWifi();
    WiFi.begin(settings_.ssid.c_str(), settings_.password.c_str());
    lastWifiReconnectAttemptTimeMs_ = millis();
}

bool WifiConnection::startMdns()
{
    lastMdnsStartAttemptTimeMs_ = millis();

    if (hostname_.empty())
    {
        Logger::w("WifiConnection", "Failed to start mDNS: hostname is empty");
        return false;
    }

    stopMdns();

    const std::string localHostname = hostname_ + ".local";
    if (!MDNS.begin(hostname_.c_str()))
    {
        Logger::w("WifiConnection", "Failed to start mDNS responder for hostname %s", hostname_.c_str());
        return false;
    }

    mdnsResponderStarted_ = true;
    MDNS.setInstanceName(hostname_.c_str());
    Logger::i("WifiConnection", "mDNS responder started: %s/%s", hostname_.c_str(), localHostname.c_str());

    if (!MDNS.addService(DnsSdServiceName, DnsSdProtocol, HttpsApiPort))
    {
        Logger::w(
            "WifiConnection",
            "Failed to register DNS-SD service: type=%s name=%s port=%u",
            DnsSdServiceType,
            hostname_.c_str(),
            HttpsApiPort);
        stopMdns();
        return false;
    }

    bool txtRegistered =
        addServiceTxt("hostname", hostname_) &&
        addServiceTxt("localHostname", localHostname) &&
        addServiceTxt("api", "https") &&
        addServiceTxt("device", "automatic-watering-hub");
    if (!txtRegistered)
    {
        Logger::w(
            "WifiConnection",
            "Failed to register one or more DNS-SD TXT records: type=%s name=%s port=%u",
            DnsSdServiceType,
            hostname_.c_str(),
            HttpsApiPort);
        stopMdns();
        return false;
    }

    mdnsServiceRegistered_ = true;
    Logger::i(
        "WifiConnection",
        "DNS-SD service registered: type=%s name=%s port=%u",
        DnsSdServiceType,
        hostname_.c_str(),
        HttpsApiPort);
    return true;
}

bool WifiConnection::addServiceTxt(const char *key, const std::string &value)
{
    return MDNS.addServiceTxt(
        const_cast<char *>(DnsSdServiceName),
        const_cast<char *>(DnsSdProtocol),
        const_cast<char *>(key),
        const_cast<char *>(value.c_str()));
}

void WifiConnection::stopMdns()
{
    if (!mdnsResponderStarted_ && !mdnsServiceRegistered_)
    {
        return;
    }

    MDNS.end();
    mdnsResponderStarted_ = false;
    mdnsServiceRegistered_ = false;
}
