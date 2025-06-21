#ifndef CaptivePortalServer_h
#define CaptivePortalServer_h

#include <DNSServer.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev

class CaptivePortalServer
{
public:
    DNSServer *_dnsServer;
    AsyncWebServer *_server;

    CaptivePortalServer();
    void setUpDNSServer(const IPAddress &localIP);
    void tearDownDNSServer();
};

#endif