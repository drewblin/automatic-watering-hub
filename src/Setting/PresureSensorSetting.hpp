#include <stdint.h>

class PresureSensorSetting
{
public:
    PresureSensorSetting(uint8_t slaveAddress);
    uint8_t getSlaveAddress();
};
