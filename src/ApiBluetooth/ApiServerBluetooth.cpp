#include "ApiServerBluetooth.hpp"

#include <Arduino.h>
#include "Logging/Logger.hpp"
#include "NimBLEDevice.h"

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
    advertising->addServiceUUID(ServiceUuid);
    NimBLEDevice::startAdvertising();
}

void ApiServerBluetooth::loop()
{
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
