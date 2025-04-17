#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Error codes for the HTTP utilities - using distinct values to avoid overlap
enum HttpError
{
  CLIENT_ERROR = 100,    // Failed to create client
  CONNECTION_ERROR = 101 // Failed to connect
};

/**
 * @brief Higher-order function that sets up WiFiClient and initial HTTPClient, then runs a callback with both
 * @param url The initial URL to connect to
 * @param callback Function to call with the WiFiClient and HTTPClient (must return int)
 * @return The int value returned by the callback function or CLIENT_ERROR/CONNECTION_ERROR on failure
 */
template <typename Callback>
int withHttp(const String &url, Callback callback)
{
  bool isHttps = (url.indexOf("https://") != -1);

  // Conditionally allocate only the client we need
  WiFiClient *client = nullptr;

  if (isHttps)
  {
    WiFiClientSecure *secureClient = new WiFiClientSecure();
    secureClient->setInsecure();
    client = secureClient;
  }
  else
  {
    client = new WiFiClient();
  }

  // Check if client creation succeeded
  if (!client)
  {
    return CLIENT_ERROR;
  }

  int result;
  { // scoping block for http client

    HTTPClient https;
    if (!https.begin(*client, url))
    {
      delete client;
      return CONNECTION_ERROR;
    }

    // Call the callback with both the WiFiClient and HTTPClient
    result = callback(*client, https);

    // Clean up
    https.end();
  }
  delete client;

  return result;
}

#endif // HTTP_UTILS_H