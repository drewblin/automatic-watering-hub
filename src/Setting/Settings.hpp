#include <vector>
#include "ValveSetting.hpp"
#include "PresureSensorSetting.hpp"
#include "WaterCounterSetting.hpp"
#include "HumiditySensorSetting.hpp"

class Settings
{
public:
    Settings();
    std::vector<ValveSetting> getValveSetting();
    PresureSensorSetting getPresureSensorSetting();
    WaterCounterSetting getMagistralWaterCounterSetting();
    std::vector<WaterCounterSetting> getLeafWaterCounterSetting();
    std::vector<HumiditySensorSetting> getHumiditySensorSetting();
};
