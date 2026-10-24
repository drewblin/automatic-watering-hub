#include "ModbusTransaction.hpp"

ModbusTransaction::ModbusTransaction(ModbusSensorPower &power)
    : power_(power)
{
    power_.acquire();
    power_.waitUntilReady();
}

ModbusTransaction::~ModbusTransaction()
{
    power_.release();
}
