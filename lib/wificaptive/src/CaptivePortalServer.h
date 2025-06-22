#pragma once

#include <DNSServer.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include "WebServer.h"
#include "WifiCredentialStore.h"
#include "WifiConnector.h"
#include "wifi-types.h"

class CaptivePortalServer
{

private:
public:
    WifiCreds _credentials = {"", ""};
    String _api_server = "";

    DNSServer *_dnsServer;
    AsyncWebServer *_server;

    CaptivePortalServer();
    void setUpDNSServer(const IPAddress &localIP);
    void tearDown();
    void begin(WifiOperationCallbacks callbacks); // todo: name?
    bool runCaptivePortal(WifiCredentialStore *credentialStore, WifiConnector *wifiConnector);

    /// @brief Sets the connection credentials from captive portal
    void setConnectionCredentials(const WifiCreds &creds, const String &api_server);
};
