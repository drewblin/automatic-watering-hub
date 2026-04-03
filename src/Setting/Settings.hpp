#include <vector>
#include "ValveSetting.hpp"
#include "PresureSensorSetting.hpp"
#include "WaterCounterSetting.hpp"
#include "SoilSensorSetting.hpp"

class Settings
{
public:
    Settings();
    std::vector<ValveSetting> getValveSetting();
    PresureSensorSetting getPresureSensorSetting();
    WaterCounterSetting getMagistralWaterCounterSetting();
    std::vector<WaterCounterSetting> getLeafWaterCounterSetting();
    std::vector<SoilSensorSetting> getSoilSensorSetting();
};
