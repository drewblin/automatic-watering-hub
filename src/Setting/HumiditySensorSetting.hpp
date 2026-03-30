#include <stdint.h>
#include <string>

class HumiditySensorSetting
{
public:
    HumiditySensorSetting();
    uint8_t getSlaveAddress();
    std::string getName();
};
