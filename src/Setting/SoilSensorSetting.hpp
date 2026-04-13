#include <stdint.h>
#include <string>

class SoilSensorSetting
{
public:
    SoilSensorSetting(uint8_t slaveAddress, std::string name);
    uint8_t getSlaveAddress();
    std::string getName();
};
