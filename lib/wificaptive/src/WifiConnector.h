#ifndef WifiConnector_h
#define WifiConnector_h

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include "WifiCredentialStore.h"

#define CONNECTION_TIMEOUT 15000

class WifiConnector
{
public:
    WifiConnector();
    
    // Connection helper methods
    uint8_t waitForConnectResult(uint32_t timeout);
    uint8_t waitForConnectResult();

    uint8_t connect(String ssid, String pass);
    std::vector<Network> getScannedUniqueNetworks(bool runScan);

    bool tryConnectWithRetries(const String &ssid, const String &password);

    bool autoConnect(WifiCredentialStore &credentialStore);
};

#endif