#pragma once

#include "Hub/WaterHub.hpp"

class Hypervisor
{
public:
    explicit Hypervisor(WaterHub &waterHub);

    void begin();
    void loop();

private:
    WaterHub &waterHub_;

    void closeExpiredValves();
};
