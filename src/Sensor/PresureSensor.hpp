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
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;

    float convertToBar(float value, uint16_t unit);
};
