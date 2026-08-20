#pragma once

#include "Setting/WifiSettings.hpp"
#include <string>

class WifiConnection
{
public:
    WifiConnection(WifiSettings settings, std::string hostname);
    bool begin();

private:
    WifiSettings settings_;
    std::string hostname_;
};
