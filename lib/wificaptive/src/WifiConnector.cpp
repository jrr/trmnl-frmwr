#include "WifiConnector.h"
#include "WifiCredentialStore.h"
#include <trmnl_log.h>

// Define the maximum number of connection attempts
#define WIFI_CONNECTION_ATTEMPTS 3

WifiConnector::WifiConnector()
{
}

uint8_t WifiConnector::waitForConnectResult(uint32_t timeout)
{
    if (timeout == 0)
    {
        return WiFi.waitForConnectResult();
    }

    unsigned long timeoutmillis = millis() + timeout;
    uint8_t status = WiFi.status();

    while (millis() < timeoutmillis)
    {
        status = WiFi.status();
        // @todo detect additional states, connect happens, then dhcp then get ip, there is some delay here, make sure not to timeout if waiting on IP
        if (status == WL_CONNECTED || status == WL_CONNECT_FAILED)
        {
            return status;
        }
        delay(100);
    }

    return status;
}

uint8_t WifiConnector::waitForConnectResult()
{
    return waitForConnectResult(CONNECTION_TIMEOUT);
}

uint8_t WifiConnector::connect(String ssid, String pass)
{
    uint8_t connRes = (uint8_t)WL_NO_SSID_AVAIL;

    if (ssid != "")
    {
        WiFi.enableSTA(true);
        WiFi.begin(ssid.c_str(), pass.c_str());
        connRes = waitForConnectResult();
    }

    return connRes;
}

std::vector<Network> WifiConnector::getScannedUniqueNetworks(bool runScan)
{
    std::vector<Network> uniqueNetworks;
    int n = WiFi.scanComplete();
    if (runScan == true)
    {
        WiFi.scanNetworks(false);
        delay(100);
        int n = WiFi.scanComplete();
        while (n == WIFI_SCAN_RUNNING || n == WIFI_SCAN_FAILED)
        {
            delay(100);
            if (n == WIFI_SCAN_RUNNING)
            {
                n = WiFi.scanComplete();
            }
            else if (n == WIFI_SCAN_FAILED)
            {
                // There is a race coniditon that can occur, particularly if you use the async flag of WiFi.scanNetworks(true),
                // where you can race before the data is parsed. scanComplete will be -2, we'll see that and fail out, but then a few microseconds later it actually
                // fills in. This fixes that, in case we ever move back to the async version of scanNetworks, but as long as it's sync above it'll work
                // first shot always.
                Log_verbose("Supposedly failed to finish scan, let's wait 10 seconds before checking again");
                delay(10000);
                n = WiFi.scanComplete();
                if (n > 0)
                {
                    Log_verbose("Scan actually did complete, we have %d networks, breaking loop.", n);
                    // it didn't actually fail, we just raced before the scan was done filling in data
                    break;
                }
                WiFi.scanNetworks(false);
                delay(500);
                n = WiFi.scanComplete();
            }
        }
    }

    n = WiFi.scanComplete();
    Log_verbose("Scanning networks, final scan result: %d", n);

    // Process each found network
    for (int i = 0; i < n; ++i)
    {
        if (!WiFi.SSID(i).equals("TRMNL"))
        {
            String ssid = WiFi.SSID(i);
            int32_t rssi = WiFi.RSSI(i);
            bool open = WiFi.encryptionType(i);
            bool found = false;
            for (auto &network : uniqueNetworks)
            {
                if (network.ssid == ssid)
                {
                    Serial.println("Equal SSID");
                    found = true;
                    if (network.rssi < rssi)
                    {
                        network.rssi = rssi; // Update to higher RSSI
                    }
                    break;
                }
            }
            if (!found)
            {
                uniqueNetworks.push_back({ssid, rssi, open});
            }
        }
    }

    Log_info("Unique networks found: %d", uniqueNetworks.size());
    for (auto &network : uniqueNetworks)
    {
        Log_info("SSID: %s, RSSI: %d, Open: %d", network.ssid.c_str(), network.rssi, network.open);
    }

    return uniqueNetworks;
}

bool WifiConnector::tryConnectWithRetries(const String &ssid, const String &password)
{
    for (int attempt = 0; attempt < WIFI_CONNECTION_ATTEMPTS; attempt++)
    {
        Log_info("Attempt %d to connect to %s", attempt + 1, ssid.c_str());
        connect(ssid, password);

        // Check if connected
        if (WiFi.status() == WL_CONNECTED)
        {
            Log_info("Connected to %s", ssid.c_str());
            return true;
        }
        WiFi.disconnect();

        // Exponential backoff: 2s, 4s, 8s delays between attempts
        if (attempt < WIFI_CONNECTION_ATTEMPTS - 1)
        {
            uint32_t backoff_delay = 2000 * (1 << attempt); // 2^attempt * 2000ms
            Log_info("Connection failed, waiting %d ms before retry...", backoff_delay);
            delay(backoff_delay);
        }
    }
    return false;
}

bool WifiConnector::autoConnect(WifiCredentialStore &credentialStore)
{
    Log_info("Trying to autoconnect to wifi...");
    credentialStore.readCredentials();

    // if last used network is available, try to connect to it
    if (credentialStore.hasLastUsedCredential())
    {
        WifiCreds lastUsed = credentialStore.getLastUsedCredential();
        Log_info("Trying to connect to last used %s...", lastUsed.ssid.c_str());
        WiFi.setSleep(0);
        WiFi.setMinSecurity(WIFI_AUTH_OPEN);
        WiFi.mode(WIFI_STA);

        if (tryConnectWithRetries(lastUsed.ssid, lastUsed.pswd))
        {
            return true;
        }
    }

    Log_info("Last used network unavailable, scanning for known networks...");
    std::vector<Network> scanResults = getScannedUniqueNetworks(true);
    std::vector<WifiCreds> sortedNetworks = credentialStore.getPrioritizedCredentials(scanResults);

    WiFi.mode(WIFI_STA);
    WifiCreds lastUsed = credentialStore.getLastUsedCredential();
    for (auto &network : sortedNetworks)
    {
        if (network.ssid == "" || (network.ssid == lastUsed.ssid && network.pswd == lastUsed.pswd))
        {
            continue;
        }

        Log_info("Trying to connect to saved network %s...", network.ssid.c_str());

        if (tryConnectWithRetries(network.ssid, network.pswd))
        {
            credentialStore.saveLastUsedSsid(network.ssid);
            return true;
        }
    }

    Log_info("Failed to connect to any network");
    return false;
}

bool WifiConnector::connectIfNeeded(const String &ssid, const String &password)
{
    auto status = WiFi.status();
    if (status != WL_CONNECTED)
    {
        Log_info("Not connected after AP disconnect");
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        waitForConnectResult();
        return WiFi.status() == WL_CONNECTED;
    }
    return true;
}