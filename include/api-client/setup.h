#include <types.h>
#include <api_types.h>
#include <HTTPClient.h>

struct ApiSetupInputs
{
  String baseUrl;
  String macAddress;
  String firmwareVersion;
};

struct ApiSetupResult
{
  https_request_err_e error;
  ApiSetupResponse response;
  String error_detail;
};

void addSetupHeaders(HTTPClient &https, ApiSetupInputs &inputs);

ApiSetupResult fetchApiSetup(ApiSetupInputs &inputs);