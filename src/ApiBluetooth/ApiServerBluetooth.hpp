#pragma once

#include "NimBLECharacteristic.h"
#include "Command/GetApiAccessTokenCommand.hpp"
#include "Command/GetWifiIpAddressCommand.hpp"
#include "Command/GetWifiSettingsCommand.hpp"
#include "Command/SaveWifiSettingsCommand.hpp"

class ApiServerBluetooth : public NimBLECharacteristicCallbacks
{
public:
    static constexpr char DeviceName[] = "Automatic Watering Hub";
    static constexpr char ServiceUuid[] = "4d42b2d0-35ba-4b70-b8a2-d1cf01e904c1";
    static constexpr char WifiSettingsUuid[] = "4d42b2d1-35ba-4b70-b8a2-d1cf01e904c1";
    static constexpr char SaveWifiSettingsUuid[] = "4d42b2d2-35ba-4b70-b8a2-d1cf01e904c1";
    static constexpr char WifiIpAddressUuid[] = "4d42b2d3-35ba-4b70-b8a2-d1cf01e904c1";
    static constexpr char ApiAccessTokenUuid[] = "4d42b2d4-35ba-4b70-b8a2-d1cf01e904c1";
    static constexpr char LogNotificationsUuid[] = "4d42b2d5-35ba-4b70-b8a2-d1cf01e904c1";
    static constexpr uint32_t AccessPasskey = 482917;

    ApiServerBluetooth(
        GetWifiSettingsCommand getWifiSettingsCommand,
        SaveWifiSettingsCommand saveWifiSettingsCommand,
        GetWifiIpAddressCommand getWifiIpAddressCommand,
        GetApiAccessTokenCommand getApiAccessTokenCommand);

    void begin();
    void loop();

private:
    template <typename Command>
    ApiCommandResult execute(Command &command, NimBLECharacteristic &characteristic)
    {
        JsonDocument request;
        const NimBLEAttValue &value = characteristic.getValue();
        DeserializationError parseError = deserializeJson(request, value.data(), value.size());
        if (parseError)
        {
            ApiCommandResult result(400, false);
            result.error = String("Invalid JSON: ") + parseError.c_str();
            return result;
        }

        return command.execute(request);
    }

    void onRead(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo) override;
    void onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &connInfo) override;
    void sendCommandResult(NimBLECharacteristic &characteristic, const ApiCommandResult &result);

    GetWifiSettingsCommand getWifiSettingsCommand_;
    SaveWifiSettingsCommand saveWifiSettingsCommand_;
    GetWifiIpAddressCommand getWifiIpAddressCommand_;
    GetApiAccessTokenCommand getApiAccessTokenCommand_;
    NimBLECharacteristic *wifiSettingsCharacteristic_ = nullptr;
    NimBLECharacteristic *saveWifiSettingsCharacteristic_ = nullptr;
    NimBLECharacteristic *wifiIpAddressCharacteristic_ = nullptr;
    NimBLECharacteristic *apiAccessTokenCharacteristic_ = nullptr;
    NimBLECharacteristic *logNotificationsCharacteristic_ = nullptr;
    bool restartScheduled_ = false;
};
