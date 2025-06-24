#include "WifiCaptive.h"
#include <trmnl_log.h>
#include "WebServer.h"
#include "wifi-helpers.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "events/arduino-events.h"
#include "events/idf-events.h"

static void idf_wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data)
{
    auto name = idfWifiEventName((wifi_event_t)event_id);
    Log_info("[IDF] WiFi event: %s", name.c_str());
    switch (event_id)
    {
    case WIFI_EVENT_STA_CONNECTED:
    {
        wifi_event_sta_connected_t *event = (wifi_event_sta_connected_t *)event_data;
        Log_info("   -> station connected to %s (channel: %d, authmode: %d)",
                 event->ssid, event->channel, event->authmode);
        break;
    }
    case WIFI_EVENT_STA_DISCONNECTED:
    {
        wifi_event_sta_disconnected_t *event = (wifi_event_sta_disconnected_t *)event_data;
        Log_info("   -> station disconnected from %s (reason: %d)",
                 event->ssid, event->reason);
        break;
    }
    case WIFI_EVENT_STA_AUTHMODE_CHANGE:
    {
        wifi_event_sta_authmode_change_t *event = (wifi_event_sta_authmode_change_t *)event_data;
        Log_info("   -> authmode changed from %d to %d", event->old_mode, event->new_mode);
        break;
    }
    }
}

static void idf_ip_event_handler(void *arg, esp_event_base_t event_base,
                                 int32_t event_id, void *event_data)
{

    auto name = idfIpEventName((ip_event_t)event_id);
    Log_info("[IDF] IP event: %s", name.c_str());
    switch (event_id)
    {
    case IP_EVENT_STA_GOT_IP:
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        Log_info("   ->" IPSTR, IP2STR(&event->ip_info.ip));
        break;
    }
    }
}

wl_status_t initiateConnectionAndWaitForOutcome(const WifiCredentials credentials)
{
    // Register IDF event handlers for detailed connection logging
    esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &idf_wifi_event_handler, NULL);
    esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &idf_ip_event_handler, NULL);

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

    // Clean up IDF event handlers
    esp_event_handler_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, &idf_wifi_event_handler);
    esp_event_handler_unregister(IP_EVENT, ESP_EVENT_ANY_ID, &idf_ip_event_handler);

    return result;
}