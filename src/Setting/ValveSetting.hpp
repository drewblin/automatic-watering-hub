#include <stdint.h>
#include <string>

class ValveSetting
{
public:
    ValveSetting();
    uint8_t getPin();
    std::string getName();
    uint8_t getHumiditySlaveAddress();
};
