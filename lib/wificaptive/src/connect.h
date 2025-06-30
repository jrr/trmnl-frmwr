#include <WiFiType.h>
#include "wifi-types.h"

wl_status_t initiateConnectionAndWaitForOutcome(const WifiCredentials credentials);
wl_status_t waitForConnectResult(uint32_t timeout);