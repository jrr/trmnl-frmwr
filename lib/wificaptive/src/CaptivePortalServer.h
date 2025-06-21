#pragma once

#include <DNSServer.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include "WebServer.h"
#include "WifiCredentialStore.h"
#include "WifiConnector.h"

class CaptivePortalServer
{

public:
    String _ssid = ""; // todo: move
    String _password = "";
    String _api_server = "";

    DNSServer *_dnsServer;
    AsyncWebServer *_server;

    CaptivePortalServer();
    void setUpDNSServer(const IPAddress &localIP);
    void tearDownServers();
    void begin(WifiOperationCallbacks callbacks); // todo: name?
    bool runCaptivePortal(WifiCredentialStore *credentialStore, WifiConnector *wifiConnector);

    /// @brief Sets the connection credentials from captive portal
    void setConnectionCredentials(const String &ssid, const String &password, const String &api_server);
};
