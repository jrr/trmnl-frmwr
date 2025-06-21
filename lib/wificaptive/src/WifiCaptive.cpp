#include "WifiCaptive.h"
#include <trmnl_log.h>
#include "WebServer.h"

bool WifiCaptive::startPortal()
{
    WifiOperationCallbacks callbacks = {
        .resetSettings = [this](bool runCallback)
        { this->resetSettings(runCallback); },
        .setConnectionCredentials = [this](const String &ssid, const String &password, const String &api_server)
        { this->_captivePortalServer.setConnectionCredentials(ssid, password, api_server); },
        .getAnnotatedNetworks = [this](bool runScan)
        { return this->getAnnotatedNetworks(runScan); }};

    _captivePortalServer.begin(callbacks);

    _credentialStore.readCredentials();

    // wait until SSID is provided
    auto succesfullyConnected = _captivePortalServer.runCaptivePortal(&_credentialStore, &_wifiConnector);

    moreStuff();

    _captivePortalServer.tearDownServers();

    return succesfullyConnected;
}

void WifiCaptive::moreStuff()
{
    // SSID provided, stop server
    WiFi.scanDelete();
    WiFi.softAPdisconnect(true);
    delay(1000);

    auto status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        Log_info("Not connected after AP disconnect");
        WiFi.mode(WIFI_STA);
        WiFi.begin(_captivePortalServer._ssid.c_str(), _captivePortalServer._password.c_str());
        _wifiConnector.waitForConnectResult();
    }
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

std::vector<Network> WifiCaptive::getAnnotatedNetworks(bool runScan)
{
    std::vector<Network> uniqueNetworks = _wifiConnector.getScannedUniqueNetworks(runScan);
    return _credentialStore.annotateNetworksWithSavedStatus(uniqueNetworks);
}

WifiCaptive WifiCaptivePortal;
