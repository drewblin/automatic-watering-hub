#include <stdint.h>
#include "ModbusMaster.h"

class PressureSendor
{
public:
    PressureSendor(ModbusMaster &modbusNode,
                  HardwareSerial &serialPort,
                  uint8_t slaveAddress);
    float readPressure();

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &serialPort_;
    uint8_t slaveAddress_;

    float convertToBar(float value, uint16_t unit);
};
