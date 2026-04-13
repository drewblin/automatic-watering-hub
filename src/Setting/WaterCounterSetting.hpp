#include <stdint.h>
#include <string>

class WaterCounterSetting
{
public:
    WaterCounterSetting(uint8_t pin, std::string name, float litersPerTick);
    uint8_t getPin();
    float getLitersPerTick();
    std::string getName();

private:
    uint8_t pin_;
    std::string name_;
    float litersPerTick_;
};
