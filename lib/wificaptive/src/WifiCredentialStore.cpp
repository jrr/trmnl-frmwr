#include "WifiCredentialStore.h"
#include <trmnl_log.h>

WifiCredentialStore::WifiCredentialStore()
{
    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        _savedWifis[i] = {"", ""};
    }
}

void WifiCredentialStore::readCredentials()
{
    Preferences preferences;
    preferences.begin("wificaptive", true);

    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        _savedWifis[i].ssid = preferences.getString(WIFI_SSID_KEY(i), "");
        _savedWifis[i].pswd = preferences.getString(WIFI_PSWD_KEY(i), "");
    }

    preferences.end();
}
void WifiCredentialStore::saveWifiCredentials(String ssid, String pass)
{
    Log_info("Saving wifi credentials: %s", ssid.c_str());

    // Check if the credentials already exist
    for (u16_t i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        if (_savedWifis[i].ssid == ssid && _savedWifis[i].pswd == pass)
        {
            return; // Avoid saving duplicate networks
        }
    }

    for (u16_t i = WIFI_MAX_SAVED_CREDS - 1; i > 0; i--)
    {
        _savedWifis[i] = _savedWifis[i - 1];
    }

    _savedWifis[0] = {ssid, pass};

    Preferences preferences;
    preferences.begin("wificaptive", false);
    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        preferences.putString(WIFI_SSID_KEY(i), _savedWifis[i].ssid);
        preferences.putString(WIFI_PSWD_KEY(i), _savedWifis[i].pswd);
    }
    preferences.putInt(WIFI_LAST_INDEX, 0);
    preferences.end();
}

void WifiCredentialStore::clearSavedWifiCredentials()
{

    Preferences preferences;
    preferences.begin("wificaptive", false);
    preferences.remove(WIFI_LAST_INDEX);
    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        preferences.remove(WIFI_SSID_KEY(i));
        preferences.remove(WIFI_PSWD_KEY(i));
    }
    preferences.end();

    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        _savedWifis[i] = {"", ""};
    }
}

void WifiCredentialStore::saveLastUsedWifiIndex(int index)
{
    Preferences preferences;
    preferences.begin("wificaptive", false);

    // if index is out of bounds, set to 0
    if (index < 0 || index >= WIFI_MAX_SAVED_CREDS)
    {
        index = 0;
    }

    // if index is greater than the total number of saved wifis, set to 0
    if (index > 0)
    {
        readCredentials();
        if (_savedWifis[index].ssid == "")
        {
            index = 0;
        }
    }

    preferences.putInt(WIFI_LAST_INDEX, index);
}

int WifiCredentialStore::readLastUsedWifiIndex()
{
    Preferences preferences;
    preferences.begin("wificaptive", true);
    int index = preferences.getInt(WIFI_LAST_INDEX, 0);
    // if index is out of range, return 0
    if (index < 0 || index >= WIFI_MAX_SAVED_CREDS)
    {
        index = 0;
    }

    // if index is greater than the total number of saved wifis, set to 0
    if (index > 0)
    {
        readCredentials();
        if (_savedWifis[index].ssid == "")
        {
            index = 0;
        }
    }
    preferences.end();
    return index;
}

void WifiCredentialStore::saveApiServer(String url)
{
    // if not URL is provided, don't save a preference and fall back to API_BASE_URL in config.h
    if (url == "")
        return;
    Preferences preferences;
    preferences.begin("data", false);
    preferences.putString("api_url", url);
    preferences.end();
}

void WifiCredentialStore::clearSavedApiUrl()
{
    Preferences preferences;
    preferences.begin("data", false);
    preferences.remove("api_url");
    preferences.end();
}
