#include <api-client/display.h>
#include <ArduinoLog.h>
#include <HTTPClient.h>
#include <trmnl_log.h>
#include <WiFiClientSecure.h>
#include <config.h>
#include <api_response_parsing.h>

void addHeaders(HTTPClient &https, ApiDisplayInputs &inputs)
{
  Log_info("Added headers:\n\r"
           "ID: %s\n\r"
           "Special function: %d\n\r"
           "Access-Token: %s\n\r"
           "Refresh_Rate: %s\n\r"
           "Battery-Voltage: %s\n\r"
           "FW-Version: %s\r\n"
           "RSSI: %s\r\n",
           __FILE__, __LINE__,
           inputs.macAddress.c_str(),
           inputs.specialFunction,
           inputs.apiKey.c_str(),
           String(inputs.refreshRate).c_str(),
           String(inputs.batteryVoltage).c_str(),
           inputs.firmwareVersion.c_str(),
           String(inputs.rssi));

  https.addHeader("ID", inputs.macAddress);
  https.addHeader("Access-Token", inputs.apiKey);
  https.addHeader("Refresh-Rate", String(inputs.refreshRate));
  https.addHeader("Battery-Voltage", String(inputs.batteryVoltage));
  https.addHeader("FW-Version", inputs.firmwareVersion);
  https.addHeader("RSSI", String(inputs.rssi));
  https.addHeader("Width", String(inputs.displayWidth));
  https.addHeader("Height", String(inputs.displayHeight));

  if (inputs.specialFunction != SF_NONE)
  {
    Log.info("%s [%d]: Add special function: true (%d)\r\n", __FILE__, __LINE__, inputs.specialFunction);
    https.addHeader("special_function", "true");
  }
}

ApiDisplayResult fetchApiDisplay(ApiDisplayInputs &apiDisplayInputs)
{
  WiFiClientSecure *secureClient = new WiFiClientSecure;
  secureClient->setInsecure();
  WiFiClient *insecureClient = new WiFiClient;

  bool isHttps = true;
  if (apiDisplayInputs.baseUrl.indexOf("https://") == -1)
  {
    isHttps = false;
  }

  // define client depending on the isHttps variable
  WiFiClient *client = isHttps ? secureClient : insecureClient;

  if (!client)
  {
    Log.error("%s [%d]: Unable to create client\r\n", __FILE__, __LINE__);
    return {.error = HTTPS_UNABLE_TO_CONNECT, .response = {}, .error_detail = "Unable to create client"};
  }

  HTTPClient https;
  Log.info("%s [%d]: RSSI: %d\r\n", __FILE__, __LINE__, WiFi.RSSI());
  Log.info("%s [%d]: [HTTPS] begin /api/display/ ...\r\n", __FILE__, __LINE__);
  char new_url[200];
  strcpy(new_url, apiDisplayInputs.baseUrl.c_str());
  strcat(new_url, "/api/display/");

  Log.info("%s [%d]: [HTTPS] URL: %s\r\n", __FILE__, __LINE__, new_url);

  String fw_version = String(FW_MAJOR_VERSION) + "." + String(FW_MINOR_VERSION) + "." + String(FW_PATCH_VERSION);

  if (!https.begin(*client, new_url))
  {
    Log.error("%s [%d]: [HTTPS] Unable to connect\r\n", __FILE__, __LINE__);
    return {.error = HTTPS_UNABLE_TO_CONNECT, .response = {}, .error_detail = "unable to begin() http"};
  }

  // HTTPS
  Log.info("%s [%d]: [HTTPS] GET...\r\n", __FILE__, __LINE__);
  Log.info("%s [%d]: [HTTPS] GET Route: %s\r\n", __FILE__, __LINE__, new_url);

  addHeaders(https, apiDisplayInputs);

  delay(5);

  int httpCode = https.GET();

  // httpCode will be negative on error
  if (httpCode < 0)
  {
    Log.error("%s [%d]: [HTTPS] GET... failed, error: %s\r\n", __FILE__, __LINE__, https.errorToString(httpCode).c_str());
    String errorMsg = "HTTP Client failed with error: " + https.errorToString(httpCode);
    return {.error = HTTPS_RESPONSE_CODE_INVALID, .response = {}, .error_detail = errorMsg};
  }

  // HTTP header has been send and Server response header has been handled
  Log.info("%s [%d]: GET... code: %d\r\n", __FILE__, __LINE__, httpCode);

  if (!(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY))
  {
    Log.info("%s [%d]: [HTTPS] Unable to connect\r\n", __FILE__, __LINE__);
    return {.error = HTTPS_REQUEST_FAILED, .response = {}, .error_detail = "returned code is not OK: " + String(httpCode)};
  }

  String payload = https.getString();
  size_t size = https.getSize();
  Log.info("%s [%d]: Content size: %d\r\n", __FILE__, __LINE__, size);
  Log.info("%s [%d]: Free heap size: %d\r\n", __FILE__, __LINE__, ESP.getMaxAllocHeap());
  Log.info("%s [%d]: Payload - %s\r\n", __FILE__, __LINE__, payload.c_str());

  auto apiResponse = parseResponse_apiDisplay(payload);
  if (apiResponse.outcome == ApiDisplayOutcome::DeserializationError)
  {
    return {.error = HTTPS_JSON_PARSING_ERR, .response = apiResponse, .error_detail = apiResponse.parseErrorDetail};
  }

  return {.error = HTTPS_NO_ERR, .response = apiResponse, .error_detail = ""};
}