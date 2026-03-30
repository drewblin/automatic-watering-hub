#include <stdint.h>

class PresureSendor
{
public:
    PresureSendor(uint8_t slaveAddress);
    float readPresure();
};
