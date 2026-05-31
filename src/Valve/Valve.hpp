#pragma once

#include <stdint.h>

class Valve
{
public:
    Valve(uint8_t pin);
    void open();
    void openForTime(uint32_t seconds);
    void close();
    bool isOpenExpired(uint32_t currentTimeMs) const;
    uint8_t getPin() const;

private:
    uint8_t pin_;
    bool hasCloseTime_ = false;
    uint32_t closeTimeMs_ = 0;
};
