#include "Clock.hpp"

#include <Arduino.h>
#include <esp_log.h>
#include <esp_sntp.h>

Clock *Clock::instance_ = nullptr;

void Clock::begin()
{
    instance_ = this;
    lastSuccessfulSyncTimeMs_ = millis();

    esp_sntp_set_time_sync_notification_cb(onTimeSynchronized);
    esp_sntp_set_sync_interval(syncIntervalMs_);
    configTzTime("EET-2EEST,M3.5.0/3,M10.5.0/4", "pool.ntp.org", "time.nist.gov");
}

void Clock::loop()
{
    if (millis() - lastSuccessfulSyncTimeMs_.load() <= syncFailureLogDelayMs_)
    {
        return;
    }

    if (!syncFailureLogged_.exchange(true))
    {
        ESP_LOGE("Clock", "Clock has not been synchronized with NTP for more than 48 hours");
    }
}

std::optional<std::time_t> Clock::now() const
{
    if (!hasSynchronized_.load())
    {
        return std::nullopt;
    }

    return std::time(nullptr);
}

void Clock::onTimeSynchronized(timeval *time)
{
    if (instance_ == nullptr)
    {
        return;
    }

    instance_->lastSuccessfulSyncTimeMs_ = millis();
    instance_->hasSynchronized_ = true;
    if (instance_->syncFailureLogged_.exchange(false))
    {
        ESP_LOGI("Clock", "Clock synchronization with NTP has been restored");
    }
}
