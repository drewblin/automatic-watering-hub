#include <stdint.h>

class Valve
{
public:
    Valve(uint8_t pin);
    void open();
    void close();

private:
    uint8_t pin_;
};
