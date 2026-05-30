#pragma once

#include <stdint.h>

class Valve
{
public:
    Valve(uint8_t pin);
    void open();
    void close();
    uint8_t getPin() const;

private:
    uint8_t pin_;
};
