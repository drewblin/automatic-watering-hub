#include "ApiServerBluetooth.hpp"

#include <Arduino.h>
#include "NimBLEDevice.h"

ApiServerBluetooth::ApiServerBluetooth(
    GetWifiSettingsCommand getWifiSettingsCommand,
    SaveWifiSettingsCommand saveWifiSettingsCommand,
    GetWifiIpAddressCommand getWifiIpAddressCommand)
    : getWifiSettingsCommand_(getWifiSettingsCommand),
      saveWifiSettingsCommand_(saveWifiSettingsCommand),
      getWifiIpAddressCommand_(getWifiIpAddressCommand)
{
}

void ApiServerBluetooth::begin()
{
    NimBLEDevice::init(DeviceName);
    NimBLEServer *server = NimBLEDevice::createServer();
    NimBLEService *service = server->createService(ServiceUuid);
    wifiSettingsCharacteristic_ = service->createCharacteristic(
        WifiSettingsUuid,
        NIMBLE_PROPERTY::READ);
    saveWifiSettingsCharacteristic_ = service->createCharacteristic(
        SaveWifiSettingsUuid,
        NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::WRITE);
    wifiIpAddressCharacteristic_ = service->createCharacteristic(
        WifiIpAddressUuid,
        NIMBLE_PROPERTY::READ);

    wifiSettingsCharacteristic_->setCallbacks(this);
    saveWifiSettingsCharacteristic_->setCallbacks(this);
    wifiIpAddressCharacteristic_->setCallbacks(this);

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
