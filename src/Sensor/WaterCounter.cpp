#include "WaterCounter.hpp"

WaterCounter::WaterCounter(uint8_t pin, float litersPerTick)
{
    litersPerTick_ = litersPerTick;

    pinMode(pin, INPUT_PULLDOWN);

    pcnt_unit_config_t unitConfig = {
        .low_limit = -(int)(sizeof(uint8_t) * 8),
        .high_limit = (int)(sizeof(uint8_t) * 8),
        .intr_priority = 0,
        .flags = {
            .accum_count = 1,
        },
    };

    ESP_ERROR_CHECK(pcnt_new_unit(&unitConfig, &pcntUnit_));

    pcnt_glitch_filter_config_t filterConfig = {
        .max_glitch_ns = 1000,
    };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcntUnit_, &filterConfig));

    pcnt_chan_config_t pcntChannelConfig = {
        .edge_gpio_num = pin,
        .level_gpio_num = -1,
    };

    pcnt_channel_handle_t pcntChannel = NULL;
    ESP_ERROR_CHECK(pcnt_new_channel(pcntUnit_, &pcntChannelConfig, &pcntChannel));

    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(
        pcntChannel,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,
        PCNT_CHANNEL_EDGE_ACTION_HOLD));

    pcnt_event_callbacks_t callbackConfig = {};
    ESP_ERROR_CHECK(pcnt_unit_register_event_callbacks(pcntUnit_, &callbackConfig, nullptr));

    ESP_ERROR_CHECK(pcnt_unit_enable(pcntUnit_));
    ESP_ERROR_CHECK(pcnt_unit_clear_count(pcntUnit_));
    ESP_ERROR_CHECK(pcnt_unit_start(pcntUnit_));
}

float WaterCounter::getLittersFromLastCall()
{
    int tickCount = 0;
    pcnt_unit_get_count(pcntUnit_, &tickCount);

    int delta = tickCount - lastTickCount_;

    if (tickCount > 2000000000)
    {
        pcnt_unit_clear_count(pcntUnit_);
        tickCount = 0;
    }

    lastTickCount_ = tickCount;

    return delta * litersPerTick_;
}
