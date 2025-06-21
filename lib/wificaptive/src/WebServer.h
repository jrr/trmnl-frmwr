#pragma once
#include <ESPAsyncWebServer.h>
#include <functional>
#include <vector>
#include <AsyncJson.h>
#include "WifiCaptivePage.h"
#include "wifi-types.h"

#define LocalIPURL "http://4.3.2.1"

struct WifiOperationCallbacks {
    // The three operations WebServer actually needs
    std::function<void(bool)> resetSettings;
    std::function<void(const String&, const String&, const String&)> setConnectionCredentials;
    std::function<std::vector<Network>(bool)> getAnnotatedNetworks;
};

void setUpWebserver(AsyncWebServer &server, WifiOperationCallbacks callbacks, const IPAddress &localIP);