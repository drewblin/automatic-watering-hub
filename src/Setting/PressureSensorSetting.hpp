#include <stdint.h>

class PressureSensorSetting
{
public:
    PressureSensorSetting(uint8_t slaveAddress);
    uint8_t getSlaveAddress();
};
