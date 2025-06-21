#pragma once

#include <Arduino.h>

struct WifiCreds
{
    String ssid;
    String pswd;
};

struct Network
{
    String ssid;
    int32_t rssi;
    bool open;
    bool saved;
};