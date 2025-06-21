#pragma once
#include <ESPAsyncWebServer.h>
#include "WifiCaptive.h"

void setUpWebserver(AsyncWebServer &server, WifiCaptive *wifiCaptive, const IPAddress &localIP);