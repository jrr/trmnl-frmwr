#include "WifiCredentialStore.h"
#include <trmnl_log.h>
#include <algorithm>

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

bool WifiCredentialStore::hasCredentials()
{
    return _savedWifis[0].ssid != "";
}

int WifiCredentialStore::findCredentialIndex(const String& ssid)
{
    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        if (_savedWifis[i].ssid == ssid)
        {
            return i;
        }
    }
    return -1; // not found
}

void WifiCredentialStore::saveLastUsedSsid(const String& ssid)
{
    int index = findCredentialIndex(ssid);
    if (index != -1)
    {
        saveLastUsedWifiIndex(index);
    }
}

WifiCreds WifiCredentialStore::getLastUsedCredential()
{
    int index = readLastUsedWifiIndex();
    if (index >= 0 && index < WIFI_MAX_SAVED_CREDS)
    {
        return _savedWifis[index];
    }
    return {"", ""}; // return empty credential if invalid index
}

bool WifiCredentialStore::hasLastUsedCredential()
{
    int index = readLastUsedWifiIndex();
    if (index >= 0 && index < WIFI_MAX_SAVED_CREDS)
    {
        return _savedWifis[index].ssid != "";
    }
    return false;
}

std::vector<WifiCreds> WifiCredentialStore::getPrioritizedCredentials(std::vector<Network>& scanResults)
{

    // sort scan results by RSSI
    std::sort(scanResults.begin(), scanResults.end(), [](const Network &a, const Network &b)
              { return a.rssi > b.rssi; });

    std::vector<WifiCreds> sortedWifis;
    for (auto &network : scanResults)
    {
        for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
        {
            if (network.ssid == _savedWifis[i].ssid)
            {
                sortedWifis.push_back(_savedWifis[i]);
            }
        }
    }

    // none matched, just return all the saved ones
    if (sortedWifis.empty())
    {
        Log_info("No matched networks found in scan, trying all saved networks...");
        for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
        {
            sortedWifis.push_back(_savedWifis[i]);
        }
    }

    return sortedWifis;
}

std::vector<Network> WifiCredentialStore::annotateNetworksWithSavedStatus(std::vector<Network>& scanResults)
{
    std::vector<Network> combinedNetworks;
    for (auto &network : scanResults)
    {
        bool found = false;
        for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
        {
            if (network.ssid == _savedWifis[i].ssid)
            {
                combinedNetworks.push_back({network.ssid, network.rssi, network.open, true});
                found = true;
                break;
            }
        }
        if (!found)
        {
            combinedNetworks.push_back({network.ssid, network.rssi, network.open, false});
        }
    }
    // add saved wifis that are not combinedNetworks
    for (int i = 0; i < WIFI_MAX_SAVED_CREDS; i++)
    {
        bool found = false;
        for (auto &network : combinedNetworks)
        {
            if (network.ssid == _savedWifis[i].ssid)
            {
                found = true;
                break;
            }
        }
        if (!found && _savedWifis[i].ssid != "")
        {
            combinedNetworks.push_back({_savedWifis[i].ssid, -200, false, true});
        }
    }

    return combinedNetworks;
}
