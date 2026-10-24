#include "WaterHub.hpp"
#include "Setting/SettingsSnapshot.hpp"

class WaterHubBuilder
{
public:
    WaterHubBuilder(
        ModbusMaster &modbusNode,
        HardwareSerial &modbusSerialPort,
        ModbusSensorPower &modbusSensorPower);
    WaterHub build(const SettingsSnapshot &settings);

private:
    ModbusMaster &modbusNode_;
    HardwareSerial &modbusSerialPort_;
    ModbusSensorPower &modbusSensorPower_;
};
