#pragma once

#include "ModbusBusLock.hpp"
#include "ModbusSensorPower.hpp"

class ModbusTransaction
{
public:
    explicit ModbusTransaction(ModbusSensorPower &power);
    ~ModbusTransaction();

    ModbusTransaction(const ModbusTransaction &) = delete;
    ModbusTransaction &operator=(const ModbusTransaction &) = delete;

private:
    ModbusBusLock busLock_;
    ModbusSensorPower &power_;
};
