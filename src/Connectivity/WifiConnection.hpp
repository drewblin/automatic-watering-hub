#pragma once

#include "Setting/WifiSettings.hpp"

class WifiConnection
{
public:
    explicit WifiConnection(WifiSettings settings);
    bool begin();

private:
    WifiSettings settings_;
};
