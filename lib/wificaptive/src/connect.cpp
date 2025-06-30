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