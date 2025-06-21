#ifndef WifiCredentialStore_h
#define WifiCredentialStore_h

#include <Arduino.h>
#include <Preferences.h>

#define WIFI_MAX_SAVED_CREDS 5
#define WIFI_SSID_KEY(i) ("wifi_" + String(i) + "_ssid").c_str()
#define WIFI_PSWD_KEY(i) ("wifi_" + String(i) + "_pswd").c_str()

#define WIFI_LAST_INDEX "wifi_last_index"

struct WifiCreds
{
    String ssid;
    String pswd;
};

class WifiCredentialStore
{
public:
    WifiCreds _savedWifis[WIFI_MAX_SAVED_CREDS];
    
    WifiCredentialStore();
    void readCredentials();
    bool hasCredentials();
    int findCredentialIndex(const String &ssid);

    void saveWifiCredentials(String ssid, String pass);

    void clearSavedWifiCredentials();

    int readLastUsedWifiIndex();
    void saveLastUsedSsid(const String &ssid);

    void saveApiServer(String url);
    void clearSavedApiUrl();

private:
    void saveLastUsedWifiIndex(int index);
};

#endif