#ifndef CaptivePortalServer_h
#define CaptivePortalServer_h

#include <DNSServer.h>

class CaptivePortalServer
{
public:
    DNSServer *_dnsServer;
    
    CaptivePortalServer();
    void setUpDNSServer(const IPAddress &localIP);
    void tearDownDNSServer();
};

#endif