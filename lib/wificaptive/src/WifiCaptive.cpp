#include "WifiCaptive.h"
#include <trmnl_log.h>
#include "WebServer.h"

bool WifiCaptive::startPortal()
{
    _captivePortalServer._dnsServer = new DNSServer();
    _captivePortalServer._server = new AsyncWebServer(80);

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
    _captivePortalServer.setUpDNSServer(localIP);
    
    WifiOperationCallbacks callbacks = {
        .resetSettings = [this](bool runCallback) { this->resetSettings(runCallback); },
        .setConnectionCredentials = [this](const String& ssid, const String& password, const String& api_server) { 
            this->setConnectionCredentials(ssid, password, api_server); 
        },
        .getAnnotatedNetworks = [this](bool runScan) { return this->getAnnotatedNetworks(runScan); }
    };
    
    setUpWebserver(*(_captivePortalServer._server), callbacks, localIP);

    // begin serving
    _captivePortalServer._server->begin();

    // start async network scan
    WiFi.scanNetworks(true);

    _credentialStore.readCredentials();

    bool succesfullyConnected = false;
    // wait until SSID is provided
    while (1)
    {
        _captivePortalServer._dnsServer->processNextRequest();

        if (_ssid == "")
        {
            delay(DNS_INTERVAL);
        }
        else
        {
            bool res = _wifiConnector.connect(_ssid, _password) == WL_CONNECTED;
            if (res)
            {
                _credentialStore.saveWifiCredentials(_ssid, _password);
                _credentialStore.saveApiServer(_api_server);
                succesfullyConnected = true;
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

    // SSID provided, stop server
    WiFi.scanDelete();
    WiFi.softAPdisconnect(true);
    delay(1000);

    auto status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        Log_info("Not connected after AP disconnect");
        WiFi.mode(WIFI_STA);
        WiFi.begin(_ssid.c_str(), _password.c_str());
        _wifiConnector.waitForConnectResult();
    }

    _captivePortalServer.tearDownDNSServer();
    _captivePortalServer.tearDownWebServer();

    return succesfullyConnected;
}

void WifiCaptive::resetSettings(bool runCallback)
{
    _credentialStore.clearSavedWifiCredentials();
    _credentialStore.clearSavedApiUrl();

    WiFi.disconnect(true, true);
    
    if (runCallback && _resetcallback != nullptr) {
        _resetcallback();
    }
}

void WifiCaptive::setResetSettingsCallback(std::function<void()> func)
{
    _resetcallback = func;
}

bool WifiCaptive::isSaved()
{
    _credentialStore.readCredentials();
    return _credentialStore.hasCredentials();
}

bool WifiCaptive::autoConnect()
{
    return _wifiConnector.autoConnect(_credentialStore);
}

void WifiCaptive::setConnectionCredentials(const String& ssid, const String& password, const String& api_server)
{
    _ssid = ssid;
    _password = password;
    _api_server = api_server;
}

std::vector<Network> WifiCaptive::getAnnotatedNetworks(bool runScan)
{
    std::vector<Network> uniqueNetworks = _wifiConnector.getScannedUniqueNetworks(runScan);
    return _credentialStore.annotateNetworksWithSavedStatus(uniqueNetworks);
}

WifiCaptive WifiCaptivePortal;
