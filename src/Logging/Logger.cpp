#include "Logger.hpp"

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <cstdio>
#include <cstring>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

std::atomic<NimBLECharacteristic *> Logger::bleCharacteristic_{nullptr};
std::atomic<uint32_t> Logger::droppedRemoteLogCount_{0};
std::atomic<uint32_t> Logger::droppedSensorReadingCount_{0};
std::string Logger::remoteUrl_;
std::string Logger::remoteAuthorizationToken_;
void *Logger::queue_ = nullptr;
bool Logger::started_ = false;

void Logger::begin(const std::string &url, const std::string &authorizationToken)
{
    if (started_)
    {
        return;
    }

    QueueHandle_t queue = xQueueCreate(QueueLength, sizeof(DeliveryItem));
    if (queue == nullptr)
    {
        ESP_LOGE("Logger", "Failed to allocate remote log queue");
        return;
    }

    queue_ = queue;
    BaseType_t result = xTaskCreate(worker, "log-delivery", WorkerStackSize, nullptr, 1, nullptr);
    if (result != pdPASS)
    {
        ESP_LOGE("Logger", "Failed to start remote log delivery task");
        vQueueDelete(queue);
        queue_ = nullptr;
        return;
    }

    remoteUrl_ = url;
    remoteAuthorizationToken_ = authorizationToken;
    started_ = true;
}

void Logger::setBleCharacteristic(NimBLECharacteristic *characteristic)
{
    bleCharacteristic_ = characteristic;
}

void Logger::e(const char *tag, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write(ESP_LOG_ERROR, tag, format, arguments);
    va_end(arguments);
}

void Logger::w(const char *tag, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write(ESP_LOG_WARN, tag, format, arguments);
    va_end(arguments);
}

void Logger::i(const char *tag, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write(ESP_LOG_INFO, tag, format, arguments);
    va_end(arguments);
}

void Logger::d(const char *tag, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write(ESP_LOG_DEBUG, tag, format, arguments);
    va_end(arguments);
}

void Logger::v(const char *tag, const char *format, ...)
{
    va_list arguments;
    va_start(arguments, format);
    write(ESP_LOG_VERBOSE, tag, format, arguments);
    va_end(arguments);
}

uint32_t Logger::droppedRemoteLogCount()
{
    return droppedRemoteLogCount_.load();
}

uint32_t Logger::droppedSensorReadingCount()
{
    return droppedSensorReadingCount_.load();
}

void Logger::sendSensorReading(uint8_t sensorId, const char *sensorType, const char *name, float value)
{
    DeliveryItem item = {};
    item.type = DeliveryType::SensorReading;
    item.sensorReading.sensorId = sensorId;
    item.sensorReading.uptimeMs = millis();
    item.sensorReading.value = value;

    const char *safeSensorType = sensorType == nullptr ? "" : sensorType;
    const char *safeName = name == nullptr ? "" : name;
    std::snprintf(item.sensorReading.sensorType, sizeof(item.sensorReading.sensorType), "%s", safeSensorType);
    std::snprintf(item.sensorReading.name, sizeof(item.sensorReading.name), "%s", safeName);

    QueueHandle_t queue = static_cast<QueueHandle_t>(queue_);
    if (queue != nullptr && xQueueSend(queue, &item, 0) != pdTRUE)
    {
        droppedSensorReadingCount_.fetch_add(1);
    }
}

void Logger::write(esp_log_level_t level, const char *tag, const char *format, va_list arguments)
{
    const char *safeTag = tag == nullptr ? "" : tag;
    const char *safeFormat = format == nullptr ? "" : format;
    va_list localArguments;
    va_copy(localArguments, arguments);
    esp_log_writev(level, safeTag, safeFormat, localArguments);
    va_end(localArguments);

    DeliveryItem item = {};
    item.type = DeliveryType::Log;
    item.record.level = level;
    item.record.uptimeMs = millis();
    std::snprintf(item.record.tag, sizeof(item.record.tag), "%s", safeTag);
    std::vsnprintf(item.record.message, sizeof(item.record.message), safeFormat, arguments);

    QueueHandle_t queue = static_cast<QueueHandle_t>(queue_);
    if (queue != nullptr && xQueueSend(queue, &item, 0) != pdTRUE)
    {
        droppedRemoteLogCount_.fetch_add(1);
    }
}

void Logger::worker(void *)
{
    DeliveryItem item;
    while (true)
    {
        QueueHandle_t queue = static_cast<QueueHandle_t>(queue_);
        if (queue != nullptr && xQueueReceive(queue, &item, portMAX_DELAY) == pdTRUE)
        {
            deliver(item);
        }
    }
}

void Logger::deliver(const DeliveryItem &item)
{
    char payload[PayloadCapacity];

    if (item.type == DeliveryType::Log)
    {
        formatPayload(item.record, payload, sizeof(payload));
        sendBleNotification(payload);
        sendHttpsNotification(payload, LogPath);
    }
    else
    {
        formatPayload(item.sensorReading, payload, sizeof(payload));
        sendHttpsNotification(payload, MetricsPath);
    }
}

void Logger::sendBleNotification(const char *payload)
{
    NimBLECharacteristic *characteristic = bleCharacteristic_.load();
    if (characteristic == nullptr)
    {
        ESP_LOGW("Logger", "Failed to send BLE log notification: characteristic is not configured");
        return;
    }

    characteristic->setValue(payload);
    if (!characteristic->notify())
    {
        ESP_LOGW("Logger", "Failed to send BLE log notification");
    }
}

void Logger::sendHttpsNotification(char *payload, const char *path)
{
    if (remoteUrl_.empty())
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        ESP_LOGW("Logger", "Failed to send HTTPS log notification: WiFi is not connected");
        return;
    }

    WiFiClientSecure client;
    // Replace this with setCACert() when the placeholder endpoint is provisioned.
    client.setInsecure();

    HTTPClient http;
    http.setConnectTimeout(1500);
    http.setTimeout(1500);
    std::string targetUrl = buildRemoteUrl(path);
    if (!http.begin(client, targetUrl.c_str()))
    {
        ESP_LOGW("Logger", "Failed to initialize HTTPS log notification request");
        return;
    }

    http.addHeader("Content-Type", "application/json");
    http.addHeader("Authorization", String("Bearer ") + remoteAuthorizationToken_.c_str());
    int statusCode = http.POST(reinterpret_cast<uint8_t *>(payload), std::strlen(payload));
    if (statusCode <= 0 || statusCode >= 400)
    {
        ESP_LOGW("Logger", "Failed to send HTTPS log notification: status code %d", statusCode);
    }
    http.end();
}

void Logger::formatPayload(const Record &record, char *payload, std::size_t capacity)
{
    JsonDocument document;
    document["level"] = levelName(record.level);
    document["uptimeMs"] = record.uptimeMs;
    document["tag"] = record.tag;
    document["message"] = record.message;

    serializeJson(document, payload, capacity);
}

void Logger::formatPayload(const SensorReading &reading, char *payload, std::size_t capacity)
{
    JsonDocument document;
    document["sensorId"] = reading.sensorId;
    document["sensorType"] = reading.sensorType;
    document["name"] = reading.name;
    document["value"] = reading.value;
    document["uptimeMs"] = reading.uptimeMs;

    serializeJson(document, payload, capacity);
}

std::string Logger::buildRemoteUrl(const char *path)
{
    const char *safePath = path == nullptr ? "" : path;
    if (remoteUrl_.empty() || safePath[0] == '\0')
    {
        return remoteUrl_;
    }

    bool baseEndsWithSlash = remoteUrl_.back() == '/';
    bool pathStartsWithSlash = safePath[0] == '/';

    if (baseEndsWithSlash && pathStartsWithSlash)
    {
        return remoteUrl_ + (safePath + 1);
    }
    if (!baseEndsWithSlash && !pathStartsWithSlash)
    {
        return remoteUrl_ + "/" + safePath;
    }
    return remoteUrl_ + safePath;
}

const char *Logger::levelName(esp_log_level_t level)
{
    switch (level)
    {
    case ESP_LOG_ERROR:
        return "error";
    case ESP_LOG_WARN:
        return "warning";
    case ESP_LOG_INFO:
        return "info";
    case ESP_LOG_DEBUG:
        return "debug";
    default:
        return "verbose";
    }
}
