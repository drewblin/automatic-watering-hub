#pragma once

#include <string>

class OtaUpdateService
{
public:
    OtaUpdateService(const std::string &password, const std::string &hostname);

    void begin();
    void loop();

private:
    std::string password_;
    std::string hostname_;
    bool started_ = false;
};
