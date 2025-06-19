#include <Preferences.h>

struct LogStoreResult {
  enum Status
  {
    SUCCESS,
    WRITE_FAILED,
    HEAD_UPDATE_FAILED
  } status;

  const char* message;
  uint8_t slot_used;
};

LogStoreResult store_log(const char *log_buffer, size_t size, Preferences &preferences);

void gather_stored_logs(String &log, Preferences &preferences);

void clear_stored_logs(Preferences &preferences);
