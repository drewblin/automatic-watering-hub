#include "AutomaticWatering.hpp"

#include <Arduino.h>
#include <cmath>
#include <ctime>
#include "Logging/Logger.hpp"

AutomaticWatering::AutomaticWatering(WaterHub &waterHub, const SettingsSnapshot &settings, const Clock &clock)
    : waterHub_(waterHub), settings_(settings), clock_(clock)
{
    valveHasLastClosedTime_.resize(waterHub_.getValves().size(), false);
    valveLastClosedTimeMs_.resize(waterHub_.getValves().size(), 0);
}

void AutomaticWatering::loop()
{
    const uint32_t currentTimeMs = millis();

    closeActiveValveIfNeeded(currentTimeMs);

    if (activeValve_ != nullptr || !canStartWateringNow())
    {
        return;
    }

    openNextDryValve(currentTimeMs);
}

void AutomaticWatering::closeActiveValveIfNeeded(uint32_t currentTimeMs)
{
    if (activeValve_ == nullptr)
    {
        return;
    }

    if (mustStopWatering(activeValve_))
    {
        closeActiveValve(currentTimeMs, "humidity is above stop threshold");
        return;
    }

    const uint32_t elapsedSeconds = (currentTimeMs - activeValveOpenedTimeMs_) / 1000;
    if (elapsedSeconds >= settings_.globalSettings.zoneWateringDurationSeconds)
    {
        closeActiveValve(currentTimeMs, "watering duration elapsed");
    }
}

void AutomaticWatering::openNextDryValve(uint32_t currentTimeMs)
{
    const auto &valves = waterHub_.getValves();
    if (valves.empty())
    {
        return;
    }

    for (size_t offset = 0; offset < valves.size(); ++offset)
    {
        const size_t valveIndex = (nextValveIndex_ + offset) % valves.size();
        Valve *valve = valves[valveIndex].get();
        if (!needsWatering(valve) || !hasDelayElapsed(valveIndex, currentTimeMs))
        {
            continue;
        }

        valve->open();
        activeValve_ = valve;
        activeValveIndex_ = valveIndex;
        activeValveOpenedTimeMs_ = currentTimeMs;

        Logger::i("AutomaticWatering", "Opened valve pin %u", valve->getPin());
        return;
    }
}

void AutomaticWatering::closeActiveValve(uint32_t currentTimeMs, const char *reason)
{
    if (activeValve_ == nullptr)
    {
        return;
    }

    activeValve_->close();
    Logger::i("AutomaticWatering", "Closed valve pin %u: %s", activeValve_->getPin(), reason);

    if (activeValveIndex_ < valveLastClosedTimeMs_.size())
    {
        valveLastClosedTimeMs_[activeValveIndex_] = currentTimeMs;
        valveHasLastClosedTime_[activeValveIndex_] = true;
    }

    nextValveIndex_ = activeValveIndex_ + 1;
    if (nextValveIndex_ >= waterHub_.getValves().size())
    {
        nextValveIndex_ = 0;
    }

    activeValve_ = nullptr;
}

bool AutomaticWatering::canStartWateringNow() const
{
    if (settings_.globalSettings.wateringStartMode.getValue() == WateringStartMode::Value::Immediately)
    {
        return true;
    }

    const std::optional<TimeOfDay> scheduledStartTime =
        settings_.globalSettings.wateringStartMode.getScheduledStartTime();
    const std::optional<std::time_t> now = clock_.now();
    if (!scheduledStartTime.has_value() || !now.has_value())
    {
        return false;
    }

    std::tm localTime;
    if (localtime_r(&now.value(), &localTime) == nullptr)
    {
        return false;
    }

    const int currentMinutes = localTime.tm_hour * 60 + localTime.tm_min;
    const int scheduledMinutes = scheduledStartTime->hour * 60 + scheduledStartTime->minute;
    return currentMinutes >= scheduledMinutes;
}

bool AutomaticWatering::hasDelayElapsed(size_t valveIndex, uint32_t currentTimeMs) const
{
    if (valveIndex >= valveHasLastClosedTime_.size() || !valveHasLastClosedTime_[valveIndex])
    {
        return true;
    }

    const uint32_t elapsedSeconds = (currentTimeMs - valveLastClosedTimeMs_[valveIndex]) / 1000;
    return elapsedSeconds >= settings_.globalSettings.zoneWateringRetryDelaySeconds;
}

bool AutomaticWatering::needsWatering(const Valve *valve) const
{
    const SoilSensor *sensor = waterHub_.getSoilSensorForValve(valve);
    if (sensor == nullptr)
    {
        return false;
    }

    const float humidity = sensor->getLastReadHumidity();
    return !std::isnan(humidity) &&
           humidity < settings_.globalSettings.startWateringBelowHumidityPercent;
}

bool AutomaticWatering::mustStopWatering(const Valve *valve) const
{
    const SoilSensor *sensor = waterHub_.getSoilSensorForValve(valve);
    if (sensor == nullptr)
    {
        return false;
    }

    const float humidity = sensor->getLastReadHumidity();
    return !std::isnan(humidity) &&
           humidity > settings_.globalSettings.stopWateringAboveHumidityPercent;
}
