#include "WaterHub.hpp"
#include "Setting/Settings.hpp"

class WaterHubBuilder
{
public:
    WaterHubBuilder(
        ModbusMaster &modbusNode,
        HardwareSerial &modbusSerialPort);
    WaterHub build(Settings settings);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
};
