#include <stdint.h>

class WaterCounter {
public:
    WaterCounter(uint8_t pin);
    void incrementTick();
};
