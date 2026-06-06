#pragma once

#include <cstdint>
#include <vector>
#include "Clock/Clock.hpp"
#include "Hub/WaterHub.hpp"
#include "Setting/SettingsSnapshot.hpp"

class AutomaticWatering
{
public:
    AutomaticWatering(WaterHub &waterHub, const SettingsSnapshot &settings, const Clock &clock);

    void loop();

private:
    WaterHub &waterHub_;
    const SettingsSnapshot settings_;
    const Clock &clock_;

    Valve *activeValve_ = nullptr;
    size_t activeValveIndex_ = 0;
    size_t nextValveIndex_ = 0;
    uint32_t activeValveOpenedTimeMs_ = 0;
    std::vector<bool> valveHasLastClosedTime_;
    std::vector<uint32_t> valveLastClosedTimeMs_;

    void closeActiveValveIfNeeded(uint32_t currentTimeMs);
    void openNextDryValve(uint32_t currentTimeMs);
    void closeActiveValve(uint32_t currentTimeMs, const char *reason);
    bool canStartWateringNow() const;
    bool hasDelayElapsed(size_t valveIndex, uint32_t currentTimeMs) const;
    bool needsWatering(const Valve *valve) const;
    bool mustStopWatering(const Valve *valve) const;
};
