#include "ApiServerBluetooth.hpp"

#include <Arduino.h>
#include "Logging/Logger.hpp"
#include "NimBLEDevice.h"

namespace
{
constexpr uint16_t FAST_ADVERTISING_INTERVAL = 160;  // 100 ms in 0.625 ms units
constexpr uint16_t SLOW_ADVERTISING_INTERVAL = 3200; // 2 s in 0.625 ms units
}

ApiServerBluetooth::ApiServerBluetooth(
    GetWifiSettingsCommand getWifiSettingsCommand,
    SaveWifiSettingsCommand saveWifiSettingsCommand,
    GetWifiIpAddressCommand getWifiIpAddressCommand,
    GetApiAccessTokenCommand getApiAccessTokenCommand)
    : getWifiSettingsCommand_(getWifiSettingsCommand),
      saveWifiSettingsCommand_(saveWifiSettingsCommand),
      getWifiIpAddressCommand_(getWifiIpAddressCommand),
      getApiAccessTokenCommand_(getApiAccessTokenCommand)
{
}

void ApiServerBluetooth::begin()
{
    NimBLEDevice::init(DeviceName);
    NimBLEDevice::setSecurityAuth(true, true, true);
    NimBLEDevice::setSecurityPasskey(AccessPasskey);
    NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);

    NimBLEServer *server = NimBLEDevice::createServer();
    server->setCallbacks(this, false);
    NimBLEService *service = server->createService(ServiceUuid);
    wifiSettingsCharacteristic_ = service->createCharacteristic(
        WifiSettingsUuid,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN);
    saveWifiSettingsCharacteristic_ = service->createCharacteristic(
        SaveWifiSettingsUuid,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN |
            NIMBLE_PROPERTY::WRITE |
            NIMBLE_PROPERTY::WRITE_ENC |
            NIMBLE_PROPERTY::WRITE_AUTHEN);
    wifiIpAddressCharacteristic_ = service->createCharacteristic(
        WifiIpAddressUuid,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN);
    apiAccessTokenCharacteristic_ = service->createCharacteristic(
        ApiAccessTokenUuid,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN);
    logNotificationsCharacteristic_ = service->createCharacteristic(
        LogNotificationsUuid,
        NIMBLE_PROPERTY::READ |
            NIMBLE_PROPERTY::READ_ENC |
            NIMBLE_PROPERTY::READ_AUTHEN |
            NIMBLE_PROPERTY::NOTIFY);

    wifiSettingsCharacteristic_->setCallbacks(this);
    saveWifiSettingsCharacteristic_->setCallbacks(this);
    wifiIpAddressCharacteristic_->setCallbacks(this);
    apiAccessTokenCharacteristic_->setCallbacks(this);
    Logger::setBleCharacteristic(logNotificationsCharacteristic_);

    server->start();
    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->enableScanResponse(true);
    advertising->setName(DeviceName);
    advertising->addServiceUUID(ServiceUuid);
    advertisingStartedTimeMs_ = millis();
    slowAdvertising_ = false;
    startAdvertising();
    Logger::i("ApiServerBluetooth", "BLE advertising started as %s", DeviceName);
}

void ApiServerBluetooth::loop()
{
    switchToSlowAdvertisingIfDue();

    if (restartScheduled_)
    {
        delay(250);
        ESP.restart();
    }
}

void ApiServerBluetooth::onRead(NimBLECharacteristic *characteristic, NimBLEConnInfo &)
{
    if (characteristic == wifiSettingsCharacteristic_)
    {
        sendCommandResult(*characteristic, getWifiSettingsCommand_.execute());
    }
    else if (characteristic == wifiIpAddressCharacteristic_)
    {
        sendCommandResult(*characteristic, getWifiIpAddressCommand_.execute());
    }
    else if (characteristic == apiAccessTokenCharacteristic_)
    {
        sendCommandResult(*characteristic, getApiAccessTokenCommand_.execute());
    }
}

void ApiServerBluetooth::onWrite(NimBLECharacteristic *characteristic, NimBLEConnInfo &)
{
    if (characteristic != saveWifiSettingsCharacteristic_)
    {
        return;
    }

    ApiCommandResult result = execute(saveWifiSettingsCommand_, *characteristic);
    sendCommandResult(*characteristic, result);
    if (result.success)
    {
        restartScheduled_ = true;
    }
}

void ApiServerBluetooth::onDisconnect(NimBLEServer *, NimBLEConnInfo &, int)
{
    startAdvertising();
}

void ApiServerBluetooth::sendCommandResult(NimBLECharacteristic &characteristic, const ApiCommandResult &result)
{
    JsonDocument response;
    response["success"] = result.success;
    response["data"] = result.data.as<JsonVariantConst>();

    if (result.error.length() > 0)
    {
        response["error"] = result.error;
    }
    else
    {
        response["error"] = nullptr;
    }

    String payload;
    serializeJson(response, payload);
    characteristic.setValue(payload);
}

void ApiServerBluetooth::startAdvertising()
{
    setAdvertisingInterval();
    NimBLEDevice::startAdvertising();
}

void ApiServerBluetooth::switchToSlowAdvertisingIfDue()
{
    if (slowAdvertising_ || millis() - advertisingStartedTimeMs_ < FastAdvertisingDurationMs)
    {
        return;
    }

    slowAdvertising_ = true;

    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    const bool wasAdvertising = advertising->isAdvertising();
    if (wasAdvertising)
    {
        advertising->stop();
    }

    setAdvertisingInterval();

    if (wasAdvertising)
    {
        NimBLEDevice::startAdvertising();
    }
}

void ApiServerBluetooth::setAdvertisingInterval()
{
    NimBLEAdvertising *advertising = NimBLEDevice::getAdvertising();
    advertising->setAdvertisingInterval(
        slowAdvertising_
            ? SLOW_ADVERTISING_INTERVAL
            : FAST_ADVERTISING_INTERVAL);
}
