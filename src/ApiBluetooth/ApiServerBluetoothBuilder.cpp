#include "ApiServerBluetoothBuilder.hpp"

#include <memory>

ApiServerBluetoothBuilder::ApiServerBluetoothBuilder(
    const SettingsSnapshot &settingsSnapshot,
    Settings &settings)
    : settingsSnapshot_(settingsSnapshot),
      settings_(settings)
{
}

std::unique_ptr<ApiServerBluetooth> ApiServerBluetoothBuilder::build()
{
    return std::make_unique<ApiServerBluetooth>(
        GetWifiSettingsCommand(settingsSnapshot_),
        SaveWifiSettingsCommand(settings_),
        GetWifiIpAddressCommand(),
        GetApiAccessTokenCommand(settingsSnapshot_));
}
