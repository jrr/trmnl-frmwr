#include <unity.h>
#include <ArduinoJson.h>
#include <api_request_serialization.h>

ApiLogInput input = {
    .deviceStatusStamp = {
        .wifi_rssi_level = -50,
        .wifi_status = "Connected",
        .refresh_rate = 30000,
        .time_since_last_sleep = 120,
        .current_fw_version = "1.2.3",
        .special_function = "None",
        .battery_voltage = 4.2f,
        .wakeup_reason = "Timer",
        .free_heap_size = 50000,
        .max_alloc_size = 40000,
        .screen_status = {
            .current_image = "test.png",
            .current_error_message = "",
        },
    },
    .timestamp = 1609459200, // 2021-01-01 00:00:00
    .codeline = 123,
    .sourceFile = "test.cpp",
    .logMessage = "Test log message",
    .logId = 456,
    .filenameCurrent = "current.png",
    .filenameNew = "new.png",
};

String compact(String input)
{
  JsonDocument doc;
  deserializeJson(doc, input);
  String output;
  serializeJson(doc, output);
  return output;
}

void test_serializeRequest_apiLog(void)
{
  auto expected = compact(R"({
    "creation_timestamp": 1609459200,
    "device_status_stamp": {
      "wifi_rssi_level": -50,
      "wifi_status": "Connected",
      "refresh_rate": 30000,
      "time_since_last_sleep_start": 120,
      "current_fw_version": "1.2.3",
      "special_function": "None",
      "battery_voltage": 4.2,
      "wakeup_reason": "Timer",
      "free_heap_size": 50000,
      "max_alloc_size": 40000
    },
    "log_id": 456,
    "log_message": "Test log message",
    "log_codeline": 123,
    "log_sourcefile": "test.cpp",
    "additional_info": {
      "filename_current": "current.png",
      "filename_new": "new.png"
    }
  })");

  auto result = serializeRequest_apiLog(input);

  TEST_ASSERT_EQUAL_STRING(expected.c_str(), result.c_str());
}

void test_serializeRequest_apiLog_with_retry(void)
{
  auto inputWithRetry = input;
  inputWithRetry.logRetry = true;
  inputWithRetry.retryAttempt = 2;

  auto expected = compact(R"({
    "creation_timestamp": 1609459200,
    "device_status_stamp": {
      "wifi_rssi_level": -50,
      "wifi_status": "Connected",
      "refresh_rate": 30000,
      "time_since_last_sleep_start": 120,
      "current_fw_version": "1.2.3",
      "special_function": "None",
      "battery_voltage": 4.2,
      "wakeup_reason": "Timer",
      "free_heap_size": 50000,
      "max_alloc_size": 40000
    },
    "log_id": 456,
    "log_message": "Test log message",
    "log_codeline": 123,
    "log_sourcefile": "test.cpp",
    "additional_info": {
      "filename_current": "current.png",
      "filename_new": "new.png",
      "retry_attempt": 2
    }
  })");

  String result = serializeRequest_apiLog(inputWithRetry);

  TEST_ASSERT_EQUAL_STRING(expected.c_str(), result.c_str());
}

void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void process()
{
  UNITY_BEGIN();
  RUN_TEST(test_serializeRequest_apiLog);
  RUN_TEST(test_serializeRequest_apiLog_with_retry);
  UNITY_END();
}

int main(int argc, char **argv)
{
  process();
  return 0;
}