#pragma once

#include <memory>
#include "ApiServerBluetooth.hpp"
#include "Setting/Settings.hpp"

class ApiServerBluetoothBuilder
{
public:
    ApiServerBluetoothBuilder(
        const SettingsSnapshot &settingsSnapshot,
        Settings &settings);

    std::unique_ptr<ApiServerBluetooth> build();

private:
    SettingsSnapshot settingsSnapshot_;
    Settings &settings_;
};
