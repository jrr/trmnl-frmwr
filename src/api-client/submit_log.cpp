#include "api-client/submit_log.h"
#include <stdio.h>
#include "trmnl_log.h"
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <memory>
#include <http_client.h>

SubmitLogResult submitLogToApi(LogApiInput &input, const char *api_url)
{

  Log_info("[HTTPS] begin /api/log ...");

  char new_url[200];
  strcpy(new_url, api_url);
  strcat(new_url, "/api/log");

  // int httpCode;
  SubmitLogResult result;

  auto connection_result = withHttp(new_url, [&](HTTPClient &https) -> void
                                    {
                                      Log_info("[HTTPS] POST...");

                                      String payload = "{\"log\":{\"logs_array\":[" + String(input.log_buffer) + "]}}";

                                      https.addHeader("ID", WiFi.macAddress());
                                      https.addHeader("Accept", "application/json");
                                      https.addHeader("Access-Token", input.api_key);
                                      https.addHeader("Content-Type", "application/json");
                                      Log_info("Send log - %s", payload.c_str());
                                      // start connection and send HTTP header
                                      int httpCode = https.POST(payload);

                                      // httpCode will be negative on error
                                      if (httpCode < 0)
                                      {
                                        Log_error("[HTTPS] POST... failed, error: %d %s", httpCode, String(httpCode).c_str());
                                        String errorMsg = "HTTP Client failed with error: " + String(httpCode);
                                        result = {.success = false, .error_message = https.errorToString(httpCode), httpCode};
                                      }
                                      else if (httpCode != HTTP_CODE_OK && httpCode != HTTP_CODE_MOVED_PERMANENTLY && httpCode != HTTP_CODE_NO_CONTENT)
                                      {
                                        Log_error("[HTTPS] POST... failed, returned HTTP code unknown: %d %s", httpCode, https.errorToString(httpCode).c_str());
                                        result = {.success = false, .error_message = https.errorToString(httpCode), httpCode};
                                      }
                                      else
                                      {
                                        result = {.success = true, .error_message = "", httpCode};
                                      } });

  if (connection_result == HTTPCLIENT_WIFICLIENT_ERROR)
  {
    Log.error("%s [%d]: Unable to create client\r\n", __FILE__, __LINE__);
    return SubmitLogResult{.success = false, .error_message = "Unable to create client"};
  }
  else if (connection_result == HTTPCLIENT_HTTPCLIENT_ERROR)
  {
    Log.error("%s [%d]: [HTTPS] Unable to connect\r\n", __FILE__, __LINE__);
    return SubmitLogResult{.success = false, .error_message = "unable to begin() http"};
  }
  // HTTP header has been send and Server response header has been handled
  Log_info("[HTTPS] POST OK, code: %d", result.httpCode);

  return result;
}
