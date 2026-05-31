#pragma once

#include "Setting/WifiSettings.hpp"

class WifiConnection
{
public:
    explicit WifiConnection(WifiSettings settings);
    void begin();

private:
    WifiSettings settings_;
};
