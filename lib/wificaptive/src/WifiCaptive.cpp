#include "WifiCaptive.h"
#include <trmnl_log.h>
#include "WebServer.h"

bool WifiCaptive::startPortal()
{
    WifiOperationCallbacks callbacks = {
        .resetSettings = [this](bool runCallback)
        { this->resetSettings(runCallback); },
        .setConnectionCredentials = [this](const String &ssid, const String &password, const String &api_server)
        { this->setConnectionCredentials(ssid, password, api_server); },
        .getAnnotatedNetworks = [this](bool runScan)
        { return this->getAnnotatedNetworks(runScan); }};

    _captivePortalServer.begin(callbacks);

    // #!#!#! grabbing more

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

    _captivePortalServer.tearDownServers();

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
