#include <stdint.h>
#include <string>

class WaterCounterSetting
{
public:
    WaterCounterSetting();
    uint8_t getPin();
    float getLitersPerTick();
    std::string getName();
};
