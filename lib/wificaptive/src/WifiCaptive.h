#ifndef WiFiCaptive_h
#define WiFiCaptive_h

#include <AsyncTCP.h> //https://github.com/me-no-dev/AsyncTCP using the latest dev version from @me-no-dev
#include <DNSServer.h>
#include <ESPAsyncWebServer.h> //https://github.com/me-no-dev/ESPAsyncWebServer using the latest dev version from @me-no-dev
#include <esp_wifi.h>          //Used for mpdu_rx_disable android workaround
#include <AsyncJson.h>
#include "WifiCaptivePage.h"
#include <ArduinoJson.h>
#include "WifiCredentialStore.h"
#include "WifiConnector.h"

#define WIFI_SSID "TRMNL"
#define WIFI_PASSWORD NULL

// Define the DNS interval in milliseconds between processing DNS requests
#define DNS_INTERVAL 60
// Define the maximum number of clients that can connect to the server
#define MAX_CLIENTS 1
// Define the WiFi channel to be used (channel 6 in this case)
#define WIFI_CHANNEL 6
// Define the maximum number of possible saved credentials
#define WIFI_MAX_SAVED_CREDS 5
// Define the maximum number of connection attempts
#define WIFI_CONNECTION_ATTEMPTS 3
// Define max connection timeout
#define CONNECTION_TIMEOUT 15000
// Local IP URL
#define LocalIPURL "http://4.3.2.1"

#define WIFI_SSID_KEY(i) ("wifi_" + String(i) + "_ssid").c_str()
#define WIFI_PSWD_KEY(i) ("wifi_" + String(i) + "_pswd").c_str()

class WifiCaptive
{
private:
    DNSServer *_dnsServer;
    AsyncWebServer *_server;
    String _ssid = "";
    String _password = "";
    String _api_server = "";

    std::function<void()> _resetcallback;

    WifiCredentialStore _credentialStore;
    WifiConnector _wifiConnector;

    void setUpDNSServer(DNSServer &dnsServer, const IPAddress &localIP);
    void setUpWebserver(AsyncWebServer &server, const IPAddress &localIP);
    uint8_t connect(String ssid, String pass);
    std::vector<Network> getScannedUniqueNetworks(bool runScan);

public:
    /// @brief Starts WiFi configuration portal.
    /// @return True if successfully connected to provided SSID, false otherwise.
    bool startPortal();

    /// @brief Checks if any ssid is saved
    /// @return True if any ssis is saved, false otherwise
    bool isSaved();

    /// @brief Resets all saved credentials
    void resetSettings();

    /// @brief sets the function callback that is triggered when uses performs soft reset
    /// @param func reset callback
    void setResetSettingsCallback(std::function<void()> func);

    /// @brief Connects to the saved SSID with the best signal strength
    /// @return True if successfully connected to saved SSID, false otherwise.
    bool autoConnect();
};

extern WifiCaptive WifiCaptivePortal;

#endif
