#pragma once

#include <AsyncTCP.h> //https://github.com/me-no-dev/AsyncTCP using the latest dev version from @me-no-dev
#include <DNSServer.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <esp_wifi.h>          //Used for mpdu_rx_disable android workaround
#include <AsyncJson.h>
#include "WifiCaptivePage.h"
#include <ArduinoJson.h>
#include "WifiCredentialStore.h"
#include "WifiConnector.h"
#include "CaptivePortalServer.h"

// Define the maximum number of possible saved credentials
#define WIFI_MAX_SAVED_CREDS 5
// Define max connection timeout
#define CONNECTION_TIMEOUT 15000
// Local IP URL
#define LocalIPURL "http://4.3.2.1"

#define WIFI_SSID_KEY(i) ("wifi_" + String(i) + "_ssid").c_str()
#define WIFI_PSWD_KEY(i) ("wifi_" + String(i) + "_pswd").c_str()

class WifiCaptive
{
private:
    // DNSServer *_dnsServer;
    // AsyncWebServer *_server;

    CaptivePortalServer _captivePortalServer;

private:
    std::function<void()> _resetcallback;
    WifiConnector _wifiConnector;
    WifiCredentialStore _credentialStore;

public:
    /// @brief Starts WiFi configuration portal.
    /// @return True if successfully connected to provided SSID, false otherwise.
    bool startPortal();

    /// @brief Checks if any ssid is saved
    /// @return True if any ssis is saved, false otherwise
    bool isSaved();

    /// @brief Resets all saved credentials
    void resetSettings(bool runCallback = false);

    /// @brief sets the function callback that is triggered when uses performs soft reset
    /// @param func reset callback
    void setResetSettingsCallback(std::function<void()> func);

    /// @brief Connects to the saved SSID with the best signal strength
    /// @return True if successfully connected to saved SSID, false otherwise.
    bool autoConnect();

    /// @brief Gets scanned networks annotated with saved status
    std::vector<Network> getAnnotatedNetworks(bool runScan = false);
};

extern WifiCaptive WifiCaptivePortal;
