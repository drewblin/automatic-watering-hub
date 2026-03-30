#include "WaterHub.hpp"
#include "Setting/Settings.hpp"

class WaterHubBuilder
{
public:
    WaterHubBuilder() = default;
    WaterHub build(Settings settings);
};
