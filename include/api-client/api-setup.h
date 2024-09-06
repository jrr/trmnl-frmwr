#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>

enum class ApiSetupHttpOutcome
{
  Ok,
  HttpClientError,
  HttpNon200,
  StatusFieldNon200,
  ResponseParseError
};

struct ApiSetupHttpResponse
{
  ApiSetupHttpOutcome outcome;
  int httpCode;
  int status;
  String api_key;
  String friendly_id;
  String image_url;
  String message;
};

ApiSetupHttpResponse fetchApiSetup(WiFiClientSecure *client, HTTPClient &https, String macAddress);