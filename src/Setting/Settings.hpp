#include <vector>
#include "ArduinoJson.h"
#include "ValveSetting.hpp"
#include "PressureSensorSetting.hpp"
#include "WaterCounterSetting.hpp"
#include "SoilSensorSetting.hpp"

class Settings
{
public:
    Settings();
    std::vector<ValveSetting> getValveSetting();
    PressureSensorSetting getPressureSensorSetting();
    WaterCounterSetting getMagistralWaterCounterSetting();
    std::vector<WaterCounterSetting> getLeafWaterCounterSetting();
    std::vector<SoilSensorSetting> getSoilSensorSetting();

private:
    JsonDocument doc_;
};
