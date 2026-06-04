#pragma once

#include <atomic>
#include <cstdarg>
#include <cstddef>
#include <cstdint>
#include <string>
#include "NimBLECharacteristic.h"
#include "esp_log.h"

class Logger
{
public:
    static void begin(const std::string &url, const std::string &authorizationToken);
    static void setBleCharacteristic(NimBLECharacteristic *characteristic);

    static void e(const char *tag, const char *format, ...);
    static void w(const char *tag, const char *format, ...);
    static void i(const char *tag, const char *format, ...);
    static void d(const char *tag, const char *format, ...);
    static void v(const char *tag, const char *format, ...);

    static uint32_t droppedRemoteLogCount();

private:
    static constexpr std::size_t TagCapacity = 32;
    static constexpr std::size_t MessageCapacity = 224;
    static constexpr std::size_t PayloadCapacity = 640;
    static constexpr uint8_t QueueLength = 12;
    static constexpr uint32_t WorkerStackSize = 4096;

    struct Record
    {
        esp_log_level_t level;
        uint32_t uptimeMs;
        char tag[TagCapacity];
        char message[MessageCapacity];
    };

    static void write(esp_log_level_t level, const char *tag, const char *format, va_list arguments);
    static void worker(void *);
    static void deliver(const Record &record);
    static void sendBleNotification(const char *payload);
    static void sendHttpsNotification(char *payload);
    static void formatPayload(const Record &record, char *payload, std::size_t capacity);
    static const char *levelName(esp_log_level_t level);

    static std::atomic<NimBLECharacteristic *> bleCharacteristic_;
    static std::atomic<uint32_t> droppedRemoteLogCount_;
    static std::string remoteUrl_;
    static std::string remoteAuthorizationToken_;
    static void *queue_;
    static bool started_;
};
