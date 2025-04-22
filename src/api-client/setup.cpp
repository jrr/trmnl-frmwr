#include <api-client/setup.h>
#include <ArduinoLog.h>
#include <HTTPClient.h>
#include <trmnl_log.h>
#include <WiFiClientSecure.h>
#include <config.h>
#include <api_response_parsing.h>
#include <http_client.h>

void addSetupHeaders(HTTPClient &https, ApiSetupInputs &inputs)
{
  Log.info("Added setup headers:\n\r"
           "ID: %s\n\r"
           "FW-Version: %s\r\n",
           inputs.macAddress.c_str(),
           inputs.firmwareVersion.c_str());

  https.addHeader("ID", inputs.macAddress);
  https.addHeader("FW-Version", inputs.firmwareVersion);
}

ApiSetupResult fetchApiSetup(ApiSetupInputs &inputs)
{
  return withHttp(
      inputs.baseUrl + "/api/setup/",
      [&inputs](HTTPClient *https, HttpError error) -> ApiSetupResult
      {
        if (error == HttpError::HTTPCLIENT_WIFICLIENT_ERROR)
        {
          Log.error("%s [%d]: Unable to create WiFiClient\r\n", __FILE__, __LINE__);
          return ApiSetupResult{
              .error = https_request_err_e::HTTPS_UNABLE_TO_CONNECT,
              .response = {},
              .error_detail = "Unable to create WiFiClient",
          };
        }
        if (error == HttpError::HTTPCLIENT_HTTPCLIENT_ERROR)
        {
          Log.error("%s [%d]: Unable to create HTTPClient\r\n", __FILE__, __LINE__);
          return ApiSetupResult{
              .error = https_request_err_e::HTTPS_UNABLE_TO_CONNECT,
              .response = {},
              .error_detail = "Unable to create HTTPClient",
          };
        }

        addSetupHeaders(*https, inputs);

        delay(5);

        int httpCode = https->GET();

        if (httpCode < 0 ||
            !(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY))
        {
          Log.error("%s [%d]: [HTTPS] GET... failed, error: %s\r\n", __FILE__, __LINE__, https->errorToString(httpCode).c_str());

          return ApiSetupResult{
              .error = https_request_err_e::HTTPS_RESPONSE_CODE_INVALID,
              .response = {},
              .error_detail = "HTTP Client failed with error: " + https->errorToString(httpCode) +
                              "(" + String(httpCode) + ")"};
        }

        // HTTP header has been send and Server response header has been handled
        Log.info("%s [%d]: GET... code: %d\r\n", __FILE__, __LINE__, httpCode);

        String payload = https->getString();
        size_t size = https->getSize();
        Log.info("%s [%d]: Content size: %d\r\n", __FILE__, __LINE__, size);
        Log.info("%s [%d]: Free heap size: %d\r\n", __FILE__, __LINE__, ESP.getMaxAllocHeap());
        Log.info("%s [%d]: Payload - %s\r\n", __FILE__, __LINE__, payload.c_str());

        auto apiResponse = parseResponse_apiSetup(payload);

        if (apiResponse.outcome == ApiSetupOutcome::DeserializationError)
        {
          return ApiSetupResult{
              .error = https_request_err_e::HTTPS_JSON_PARSING_ERR,
              .response = {},
              .error_detail = "JSON parse failed"};
        }
        else if (apiResponse.outcome == ApiSetupOutcome::StatusError)
        {
          return ApiSetupResult{
              .error = https_request_err_e::HTTPS_RESPONSE_CODE_INVALID,
              .response = apiResponse,
              .error_detail = "API returned error status: " + String(apiResponse.status)};
        }
        else
        {
          return ApiSetupResult{
              .error = https_request_err_e::HTTPS_NO_ERR,
              .response = apiResponse,
              .error_detail = ""};
        }
      });
}