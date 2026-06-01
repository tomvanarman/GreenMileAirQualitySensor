#pragma once

#include <Arduino.h>
#include <HTTPClient.h>
#include "DEBUG.h"
#include <cstdio>
#include <cstring>
#include <string>

#include "mbedtls/md.h"

class HttpManager
{
public:
    std::pair<bool, std::string> signBody(const char* body, const char* secret);
    bool post(const String device_id, const String &signature, const String &url, const String &endpoint, const String &payload);

private:
    HTTPClient http;
};
