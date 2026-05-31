#include <Arduino.h>
#include <cassert>
#include "Valve.hpp"

Valve::Valve(uint8_t pin) : pin_(pin)
{
    pinMode(pin_, OUTPUT);
}

void Valve::open()
{
    digitalWrite(pin_, HIGH);
    hasCloseTime_ = false;
}

void Valve::openForTime(uint32_t seconds)
{
    assert(seconds <= INT32_MAX / 1000);

    digitalWrite(pin_, HIGH);
    closeTimeMs_ = millis() + seconds * 1000;
    hasCloseTime_ = true;
}

void Valve::close()
{
    digitalWrite(pin_, LOW);
    hasCloseTime_ = false;
}

bool Valve::isOpenExpired(uint32_t currentTimeMs) const
{
    return hasCloseTime_ &&
           static_cast<int32_t>(currentTimeMs - closeTimeMs_) >= 0;
}

uint8_t Valve::getPin() const
{
    return pin_;
}
