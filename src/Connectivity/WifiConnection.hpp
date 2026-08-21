#pragma once

#include "Setting/WifiSettings.hpp"
#include <string>

class WifiConnection
{
public:
    WifiConnection(WifiSettings settings, std::string hostname);
    bool begin();
    void loop();

private:
    static constexpr uint16_t HttpsApiPort = 443;
    static constexpr char DnsSdServiceName[] = "automatic-watering";
    static constexpr char DnsSdServiceType[] = "_automatic-watering._tcp.";
    static constexpr char DnsSdProtocol[] = "tcp";
    static constexpr uint32_t WifiConnectTimeoutMs = 15000;
    static constexpr uint32_t WifiReconnectIntervalMs = 15000;
    static constexpr uint32_t MdnsRetryIntervalMs = 15000;

    void configureWifi();
    void startWifi();
    bool startMdns();
    bool addServiceTxt(const char *key, const std::string &value);
    void stopMdns();

    WifiSettings settings_;
    std::string hostname_;
    bool wifiConnected_ = false;
    bool mdnsResponderStarted_ = false;
    bool mdnsServiceRegistered_ = false;
    uint32_t lastWifiReconnectAttemptTimeMs_ = 0;
    uint32_t lastMdnsStartAttemptTimeMs_ = 0;
};
