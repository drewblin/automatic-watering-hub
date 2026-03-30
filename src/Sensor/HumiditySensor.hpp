#include <stdint.h>

class HumiditySensor
{
public:
    HumiditySensor(uint8_t slaveAddress);
    float readHumidity();
};
