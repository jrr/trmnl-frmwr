
#include <WString.h>

struct LogApiInput
{
  String api_key;
  const char *log_buffer;
};

struct SubmitLogResult
{
  bool success;
  String error_message;
  int httpCode;
};

SubmitLogResult submitLogToApi(LogApiInput &input, const char *api_url);