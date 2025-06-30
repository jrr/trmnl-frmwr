#include "WifiCaptive.h"
#include <trmnl_log.h>
#include "WebServer.h"
#include "wifi-helpers.h"
#include "esp_event.h"
#include "esp_wifi.h"

wl_status_t initiateConnectionAndWaitForOutcome(const WifiCredentials credentials)
{
    for (int i = ARDUINO_EVENT_WIFI_READY; i < ARDUINO_EVENT_MAX; i++)
    {
        WiFi.onEvent([i](WiFiEvent_t event, WiFiEventInfo_t info)
                     { Log_info("[Arduino] WiFi event %s", WiFi.eventName((arduino_event_id_t)i)); }, (arduino_event_id_t)i);
    }

    auto beginResult = WiFi.begin(credentials.ssid.c_str(), credentials.pswd.c_str());
    Log_info("WiFi.begin() -> %s", parseWifiStatusToStr(beginResult).c_str());

    wl_status_t result = waitForConnectResult(CONNECTION_TIMEOUT);

    // Clean up Arduino event handlers
    for (int i = ARDUINO_EVENT_WIFI_READY; i < ARDUINO_EVENT_MAX; i++)
    {
        WiFi.removeEvent(i);
    }

    return result;
}

wl_status_t waitForConnectResult(uint32_t timeout)
{

    unsigned long timeoutmillis = millis() + timeout;
    wl_status_t status = WiFi.status();

    while (millis() < timeoutmillis)
    {
        wl_status_t newStatus = WiFi.status();
        if (newStatus != status)
        {
            Log_verbose("WiFi status changed from %s to %s",
                        parseWifiStatusToStr(status).c_str(),
                        parseWifiStatusToStr(newStatus).c_str());
        }
        status = newStatus;
        // @todo detect additional states, connect happens, then dhcp then get ip, there is some delay here, make sure not to timeout if waiting on IP
        if (status == WL_CONNECTED || status == WL_CONNECT_FAILED)
        {
            return status;
        }
        delay(100);
    }

    return status;
}