#ifndef WifiCredentialStore_h
#define WifiCredentialStore_h

#include <Arduino.h>
#include <Preferences.h>
#include <vector>
#include "wifi-types.h"

#define WIFI_MAX_SAVED_CREDS 5
#define WIFI_SSID_KEY(i) ("wifi_" + String(i) + "_ssid").c_str()
#define WIFI_PSWD_KEY(i) ("wifi_" + String(i) + "_pswd").c_str()

#define WIFI_LAST_INDEX "wifi_last_index"

class WifiCredentialStore
{
public:
    WifiCredentialStore();
    void readCredentials();
    bool hasCredentials();
    int findCredentialIndex(const String &ssid);
    WifiCreds getLastUsedCredential();
    bool hasLastUsedCredential();

    void saveWifiCredentials(const WifiCreds &creds);

    void clearSavedWifiCredentials();

    int readLastUsedWifiIndex();
    void saveLastUsedSsid(const String &ssid);

    void saveApiServer(String url);
    void clearSavedApiUrl();
    
    // High-level semantic methods
    std::vector<WifiCreds> getPrioritizedCredentials(std::vector<Network>& scanResults);
    std::vector<Network> annotateNetworksWithSavedStatus(std::vector<Network>& scanResults);

private:
    void saveLastUsedWifiIndex(int index);

    WifiCreds _savedWifis[WIFI_MAX_SAVED_CREDS];
};

#endif