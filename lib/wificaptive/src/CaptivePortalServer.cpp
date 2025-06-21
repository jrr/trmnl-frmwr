#include "CaptivePortalServer.h"

CaptivePortalServer::CaptivePortalServer()
{
    _dnsServer = nullptr;
}

void CaptivePortalServer::setUpDNSServer(const IPAddress &localIP)
{
    _dnsServer->setTTL(3600);
    _dnsServer->start(53, "*", localIP);
}
void CaptivePortalServer::tearDownDNSServer()
{
    _dnsServer->stop();
    delete _dnsServer;
    _dnsServer = nullptr;
}
void CaptivePortalServer::tearDownWebServer()
{
    _server->end();
    delete _server;
    _server = nullptr;
}