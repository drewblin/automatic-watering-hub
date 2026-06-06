#pragma once

#include <string>

class OtaUpdateService
{
public:
    explicit OtaUpdateService(const std::string &password);

    void begin();
    void loop();

private:
    std::string password_;
    bool started_ = false;
};
