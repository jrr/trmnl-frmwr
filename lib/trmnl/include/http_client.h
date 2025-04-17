#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

// Error codes for the HTTP utilities - using distinct values to avoid overlap
enum HttpError
{
  HTTPCLIENT_SUCCESS = 100,
  HTTPCLIENT_WIFICLIENT_ERROR = 100, // Failed to create client
  HTTPCLIENT_HTTPCLIENT_ERROR = 101  // Failed to connect
};

/**
 * @brief Higher-order function that sets up WiFiClient and HTTPClient, then runs a callback
 * @param url The initial URL to connect to
 * @param callback Function to call with the HTTPClient
 * @return HTTPCLIENT_SUCCESS / HTTPCLIENT_WIFICLIENT_ERROR / HTTPCLIENT_HTTPCLIENT_ERROR
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
    return HTTPCLIENT_WIFICLIENT_ERROR;
  }

  int result;
  { // scoping block for http client

    HTTPClient https;
    if (https.begin(*client, url))
    {

      callback(https);

      result = HTTPCLIENT_SUCCESS;

      // Clean up
      https.end();
    }
    else
    {
      result = HTTPCLIENT_HTTPCLIENT_ERROR;
    }
  }
  delete client;

  return result;
}

#endif // HTTP_UTILS_H