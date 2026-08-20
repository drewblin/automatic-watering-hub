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
    static uint32_t droppedSensorReadingCount();
    static void sendSensorReading(uint8_t sensorId, const char *sensorType, const char *name, float value);

private:
    static constexpr std::size_t TagCapacity = 32;
    static constexpr std::size_t MessageCapacity = 224;
    static constexpr std::size_t SensorTypeCapacity = 32;
    static constexpr std::size_t SensorNameCapacity = 64;
    static constexpr std::size_t PayloadCapacity = 640;
    static constexpr std::size_t FallbackBleNotificationChunkCapacity = 20;
    static constexpr uint8_t QueueLength = 24;
    static constexpr uint32_t WorkerStackSize = 4096;
    static constexpr const char *LogPath = "/logs";
    static constexpr const char *MetricsPath = "/metrics";

    enum class DeliveryType : uint8_t
    {
        Log,
        SensorReading,
    };

    struct Record
    {
        esp_log_level_t level;
        uint32_t uptimeMs;
        char tag[TagCapacity];
        char message[MessageCapacity];
    };

    struct SensorReading
    {
        uint8_t sensorId;
        uint32_t uptimeMs;
        char sensorType[SensorTypeCapacity];
        char name[SensorNameCapacity];
        float value;
    };

    struct DeliveryItem
    {
        DeliveryType type;
        Record record;
        SensorReading sensorReading;
    };

    static void write(esp_log_level_t level, const char *tag, const char *format, va_list arguments);
    static void worker(void *);
    static void deliver(const DeliveryItem &item);
    static void sendBleNotification(const char *payload);
    static bool sendBleNotificationChunks(NimBLECharacteristic &characteristic, uint16_t connHandle, const char *payload, std::size_t chunkCapacity);
    static void sendHttpsNotification(char *payload, const char *path);
    static void formatPayload(const Record &record, char *payload, std::size_t capacity);
    static void formatPayload(const SensorReading &reading, char *payload, std::size_t capacity);
    static std::string buildRemoteUrl(const char *path);
    static const char *levelName(esp_log_level_t level);

    static std::atomic<NimBLECharacteristic *> bleCharacteristic_;
    static std::atomic<uint32_t> droppedRemoteLogCount_;
    static std::atomic<uint32_t> droppedSensorReadingCount_;
    static std::string remoteUrl_;
    static std::string remoteAuthorizationToken_;
    static void *queue_;
    static bool started_;
};
