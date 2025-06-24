#include <WiFiType.h>
#include <Arduino.h>

struct WifiStatusNode
{
  const char *name;
  wl_status_t value;
};

String parseWifiStatusToStr(wl_status_t wifi_status);