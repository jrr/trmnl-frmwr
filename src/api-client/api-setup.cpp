#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <api_response_parsing.h>
#include <api-client/api-setup.h>
#include <trmnl_log.h>

ApiSetupHttpResponse fetchApiSetup(WiFiClientSecure *client, HTTPClient &https, String macAddress)
{
  https.addHeader("ID", macAddress);

  Log_info("[HTTPS] begin /api/setup ...");

  auto http_begin_success = https.begin(*client, "https://usetrmnl.com/api/setup");
  if (!http_begin_success)
  {
    return {.outcome = ApiSetupHttpOutcome::HttpClientError};
  }

  int httpCode = https.GET();

  if (httpCode != HTTP_CODE_OK)
  {
    return {.outcome = ApiSetupHttpOutcome::HttpNon200, .httpCode = httpCode};
  }

  String payload = https.getString();

  Log_info("Payload: %s", payload.c_str());

  auto apiResponse = parseResponse_apiSetup(payload);

  if (apiResponse.status != 200)
  {
    return {.outcome = ApiSetupHttpOutcome::StatusFieldNon200, .status = apiResponse.status};
  }

  if (apiResponse.outcome == ApiSetupOutcome::DeserializationError)
  {
    return {.outcome = ApiSetupHttpOutcome::ResponseParseError};
  }

  return {
      .outcome = ApiSetupHttpOutcome::Ok,
      .httpCode = httpCode,
      .status = apiResponse.status,
      .api_key = apiResponse.api_key,
      .friendly_id = apiResponse.friendly_id,
      .image_url = apiResponse.image_url,
      .message = apiResponse.message,
  };
}