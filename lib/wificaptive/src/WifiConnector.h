#ifndef WifiConnector_h
#define WifiConnector_h

#include <Arduino.h>
#include <WiFi.h>

#define CONNECTION_TIMEOUT 15000

class WifiConnector
{
public:
    WifiConnector();
    
    // Connection helper methods
    uint8_t waitForConnectResult(uint32_t timeout);
    uint8_t waitForConnectResult();
};

#endif