#include "WifiConnector.h"

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