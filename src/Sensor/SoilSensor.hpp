#include <stdint.h>
#include "ModbusMaster.h"

class SoilSensor
{
public:
    SoilSensor(
        ModbusMaster &modbusNode,
        HardwareSerial &serialPort,
        uint8_t slaveAddress);
    void readData(float &humidity, float &temperature);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;
};
