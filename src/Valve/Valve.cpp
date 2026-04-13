#include <Arduino.h>
#include "Valve.hpp"

Valve::Valve(uint8_t pin) : pin_(pin)
{
    pinMode(pin_, OUTPUT);
}

void Valve::open()
{
    digitalWrite(pin_, HIGH);
}

void Valve::close()
{
    digitalWrite(pin_, LOW);
}
