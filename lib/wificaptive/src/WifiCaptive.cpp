#include "WifiCaptive.h"
#include <trmnl_log.h>
#include "WebServer.h"

bool WifiCaptive::startPortal()
{
    _credentialStore.readCredentials();

    WifiOperationCallbacks callbacks = {
        .resetSettings = [this](bool runCallback)
        { this->resetSettings(runCallback); },
        .setConnectionCredentials = [this](const String &ssid, const String &password, const String &api_server)
        { this->_captivePortalServer.setConnectionCredentials({ssid, password}, api_server); },
        .getAnnotatedNetworks = [this](bool runScan)
        { return this->getAnnotatedNetworks(runScan); }};

    _captivePortalServer.begin(callbacks);

    // wait until SSID is provided
    auto succesfullyConnected = _captivePortalServer.runCaptivePortal(&_credentialStore, &_wifiConnector);

    if (succesfullyConnected)
    {
        _wifiConnector.connectIfNeeded(_captivePortalServer._credentials);
    }

    _captivePortalServer.tearDown();

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

std::vector<Network> WifiCaptive::getAnnotatedNetworks(bool runScan)
{
    std::vector<Network> uniqueNetworks = _wifiConnector.getScannedUniqueNetworks(runScan);
    return _credentialStore.annotateNetworksWithSavedStatus(uniqueNetworks);
}

WifiCaptive WifiCaptivePortal;
