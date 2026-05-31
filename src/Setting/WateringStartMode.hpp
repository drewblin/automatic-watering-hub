#pragma once

#include <cstring>
#include <optional>
#include "TimeOfDay.hpp"

class WateringStartMode
{
public:
    static constexpr const char *IMMEDIATELY = "immediately";
    static constexpr const char *AT_SCHEDULED_TIME = "atScheduledTime";

    enum class Value
    {
        Immediately,
        AtScheduledTime
    };

    constexpr WateringStartMode()
        : value_(Value::Immediately), scheduledStartTime_(std::nullopt)
    {
    }

    static constexpr WateringStartMode immediately()
    {
        return WateringStartMode(Value::Immediately, std::nullopt);
    }

    static constexpr WateringStartMode atScheduledTime(TimeOfDay scheduledStartTime)
    {
        return WateringStartMode(Value::AtScheduledTime, scheduledStartTime);
    }

    static std::optional<WateringStartMode> tryFrom(const char *value, TimeOfDay scheduledStartTime)
    {
        if (value != nullptr && strcmp(value, IMMEDIATELY) == 0)
        {
            return immediately();
        }

        if (value != nullptr && strcmp(value, AT_SCHEDULED_TIME) == 0)
        {
            return atScheduledTime(scheduledStartTime);
        }

        return std::nullopt;
    }

    constexpr Value getValue() const
    {
        return value_;
    }

    constexpr std::optional<TimeOfDay> getScheduledStartTime() const
    {
        return scheduledStartTime_;
    }

    constexpr const char *toString() const
    {
        return value_ == Value::Immediately ? IMMEDIATELY : AT_SCHEDULED_TIME;
    }

private:
    constexpr WateringStartMode(Value value, std::optional<TimeOfDay> scheduledStartTime)
        : value_(value), scheduledStartTime_(scheduledStartTime)
    {
    }

    Value value_;
    std::optional<TimeOfDay> scheduledStartTime_;
};
