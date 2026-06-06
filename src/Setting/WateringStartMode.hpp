#pragma once

#include <cstring>
#include <optional>
#include "TimeOfDay.hpp"

class WateringStartMode
{
public:
    static constexpr const char *IMMEDIATELY = "immediately";
    static constexpr const char *WITHIN_WATERING_WINDOW = "withinWateringWindow";

    enum class Value
    {
        Immediately,
        WithinWateringWindow
    };

    constexpr WateringStartMode()
        : value_(Value::Immediately),
          wateringWindowStartTime_(std::nullopt),
          wateringWindowEndTime_(std::nullopt)
    {
    }

    static constexpr WateringStartMode immediately()
    {
        return WateringStartMode(Value::Immediately, std::nullopt, std::nullopt);
    }

    static constexpr WateringStartMode withinWateringWindow(TimeOfDay startTime, TimeOfDay endTime)
    {
        return WateringStartMode(Value::WithinWateringWindow, startTime, endTime);
    }

    static std::optional<WateringStartMode> tryFrom(const char *value, TimeOfDay startTime, TimeOfDay endTime)
    {
        if (value != nullptr && strcmp(value, IMMEDIATELY) == 0)
        {
            return immediately();
        }

        if (value != nullptr && strcmp(value, WITHIN_WATERING_WINDOW) == 0)
        {
            return withinWateringWindow(startTime, endTime);
        }

        return std::nullopt;
    }

    constexpr Value getValue() const
    {
        return value_;
    }

    constexpr std::optional<TimeOfDay> getWateringWindowStartTime() const
    {
        return wateringWindowStartTime_;
    }

    constexpr std::optional<TimeOfDay> getWateringWindowEndTime() const
    {
        return wateringWindowEndTime_;
    }

    constexpr const char *toString() const
    {
        return value_ == Value::Immediately ? IMMEDIATELY : WITHIN_WATERING_WINDOW;
    }

private:
    constexpr WateringStartMode(
        Value value,
        std::optional<TimeOfDay> wateringWindowStartTime,
        std::optional<TimeOfDay> wateringWindowEndTime)
        : value_(value),
          wateringWindowStartTime_(wateringWindowStartTime),
          wateringWindowEndTime_(wateringWindowEndTime)
    {
    }

    Value value_;
    std::optional<TimeOfDay> wateringWindowStartTime_;
    std::optional<TimeOfDay> wateringWindowEndTime_;
};
