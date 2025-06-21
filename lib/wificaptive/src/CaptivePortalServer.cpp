#include "CaptivePortalServer.h"
#include <esp_wifi.h>

#define WIFI_SSID "TRMNL"
#define WIFI_PASSWORD NULL

// Define the maximum number of clients that can connect to the server
#define MAX_CLIENTS 1
// Define the WiFi channel to be used (channel 6 in this case)
#define WIFI_CHANNEL 6
// Define the DNS interval in milliseconds between processing DNS requests
#define DNS_INTERVAL 60

CaptivePortalServer::CaptivePortalServer()
{
    _dnsServer = nullptr;
}

void CaptivePortalServer::setUpDNSServer(const IPAddress &localIP)
{
    _dnsServer->setTTL(3600);
    _dnsServer->start(53, "*", localIP);
}
void CaptivePortalServer::tearDownServers()
{
    _dnsServer->stop();
    delete _dnsServer;
    _dnsServer = nullptr;

    _server->end();
    delete _server;
    _server = nullptr;
}
void CaptivePortalServer::begin(WifiOperationCallbacks callbacks)
{
    _dnsServer = new DNSServer();
    _server = new AsyncWebServer(80);

    // Set the WiFi mode to access point and station
    WiFi.mode(WIFI_MODE_AP);

    // Define the subnet mask for the WiFi network
    const IPAddress subnetMask(255, 255, 255, 0);
    const IPAddress localIP(4, 3, 2, 1);
    const IPAddress gatewayIP(4, 3, 2, 1);

    WiFi.disconnect();
    delay(50);

    // Configure the soft access point with a specific IP and subnet mask
    WiFi.softAPConfig(localIP, gatewayIP, subnetMask);
    delay(50);

    // Start the soft access point with the given ssid, password, channel, max number of clients
    WiFi.softAP(WIFI_SSID, WIFI_PASSWORD, WIFI_CHANNEL, 0, MAX_CLIENTS);
    delay(50);

    // Disable AMPDU RX on the ESP32 WiFi to fix a bug on Android
    esp_wifi_stop();
    esp_wifi_deinit();
    wifi_init_config_t my_config = WIFI_INIT_CONFIG_DEFAULT();
    my_config.ampdu_rx_enable = false;
    esp_wifi_init(&my_config);
    esp_wifi_start();
    vTaskDelay(100 / portTICK_PERIOD_MS); // Add a small delay

    // configure DSN and WEB server
    setUpDNSServer(localIP);

    setUpWebserver(*_server, callbacks, localIP);

    // begin serving
    _server->begin();

    // start async network scan
    WiFi.scanNetworks(true);
}
bool CaptivePortalServer::runCaptivePortal(WifiCredentialStore *credentialStore, WifiConnector *wifiConnector)
{
    while (1)
    {
        _dnsServer->processNextRequest();

        if (_ssid == "")
        {
            delay(DNS_INTERVAL);
        }
        else
        {
            bool res = wifiConnector->connect(_ssid, _password) == WL_CONNECTED;
            if (res)
            {
                credentialStore->saveWifiCredentials(_ssid, _password);
                credentialStore->saveApiServer(_api_server);
                return true;
                break;
            }
            else
            {
                _ssid = "";
                _password = "";

                WiFi.disconnect();
                WiFi.enableSTA(false);
                break;
            }
        }
    }
    return false;
}

void CaptivePortalServer::setConnectionCredentials(const String &ssid, const String &password, const String &api_server)
{
    _ssid = ssid;
    _password = password;
    _api_server = api_server;
}
