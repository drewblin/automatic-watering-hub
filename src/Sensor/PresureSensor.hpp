#include <stdint.h>
#include "ModbusMaster.h"

class PresureSendor
{
public:
    PresureSendor(ModbusMaster &modbusNode,
                  HardwareSerial &serialPort,
                  uint8_t slaveAddress);
    float readPresure();

private:
    ModbusMaster &modbusNode;
    HardwareSerial &serialPort;
    uint8_t slaveAddress;

    float convertToBar(float value, uint16_t unit);
};
