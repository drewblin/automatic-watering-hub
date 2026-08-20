#include "OtaUpdateService.hpp"

#include <ArduinoOTA.h>
#include <WiFi.h>
#include "Logging/Logger.hpp"

OtaUpdateService::OtaUpdateService(const std::string &password, const std::string &hostname)
    : password_(password),
      hostname_(hostname)
{
}

void OtaUpdateService::begin()
{
    if (started_)
    {
        return;
    }

    if (WiFi.status() != WL_CONNECTED)
    {
        Logger::w("OtaUpdate", "OTA update service is disabled because WiFi is not connected");
        return;
    }

    if (password_.empty())
    {
        Logger::e("OtaUpdate", "OTA update service is disabled because password is empty");
        return;
    }

    if (!hostname_.empty())
    {
        ArduinoOTA.setHostname(hostname_.c_str());
    }
    ArduinoOTA.setPassword(password_.c_str());

    ArduinoOTA.onStart([]()
    {
        Logger::i("OtaUpdate", "OTA update started");
    });

    ArduinoOTA.onEnd([]()
    {
        Logger::i("OtaUpdate", "OTA update finished");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
    {
        if (total == 0)
        {
            return;
        }

        static uint8_t lastLoggedPercent = 255;
        uint8_t percent = (progress * 100) / total;
        if (percent == 100 || percent / 10 != lastLoggedPercent / 10)
        {
            lastLoggedPercent = percent;
            Logger::i("OtaUpdate", "OTA update progress: %u%%", percent);
        }
    });

    ArduinoOTA.onError([](ota_error_t error)
    {
        Logger::e("OtaUpdate", "OTA update failed: %u", error);
    });

    ArduinoOTA.begin();
    started_ = true;
    Logger::i("OtaUpdate", "OTA update service started");
}

void OtaUpdateService::loop()
{
    if (started_)
    {
        ArduinoOTA.handle();
    }
}
