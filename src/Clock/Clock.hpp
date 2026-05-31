#pragma once

#include <atomic>
#include <cstdint>
#include <ctime>
#include <optional>

struct timeval;

class Clock
{
public:
    void begin();
    void loop();

    std::optional<std::time_t> now() const;

private:
    static constexpr uint32_t syncIntervalMs_ = 60 * 60 * 1000;
    static constexpr uint32_t syncFailureLogDelayMs_ = 48 * syncIntervalMs_;

    static Clock *instance_;

    std::atomic<uint32_t> lastSuccessfulSyncTimeMs_{0};
    std::atomic<bool> hasSynchronized_{false};
    std::atomic<bool> syncFailureLogged_{false};

    static void onTimeSynchronized(timeval *time);
};
