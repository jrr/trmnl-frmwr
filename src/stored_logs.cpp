#include "stored_logs.h"
#include "config.h"
#include "trmnl_log.h"
#include <ArduinoLog.h>

LogStoreResult store_log(const char *log_buffer, size_t size, Preferences &preferences)
{
  // Try to find an empty slot first
  for (uint8_t i = 0; i < LOG_MAX_NOTES_NUMBER; i++)
  {
    String key = PREFERENCES_LOG_KEY + String(i);
    if (!preferences.isKey(key.c_str()))
    {
      size_t res = preferences.putString(key.c_str(), log_buffer);
      if (res == size)
      {
        return {LogStoreResult::SUCCESS, "Log stored in new slot", i};
      }
      else
      {
        return {LogStoreResult::WRITE_FAILED, "Failed to write to new slot", i};
      }
    }
  }
  
  // All slots full, use circular buffer
  uint8_t head = preferences.getUChar(PREFERENCES_LOG_BUFFER_HEAD_KEY, 0);
  
  String key = PREFERENCES_LOG_KEY + String(head);
  size_t res = preferences.putString(key.c_str(), log_buffer);
  if (res != size)
  {
    return {LogStoreResult::WRITE_FAILED, "Failed to overwrite slot", head};
  }

  // Update head pointer
  uint8_t next_head = (head + 1) % LOG_MAX_NOTES_NUMBER;
  if (!preferences.putUChar(PREFERENCES_LOG_BUFFER_HEAD_KEY, next_head))
  {
    return {LogStoreResult::HEAD_UPDATE_FAILED, "Log written but head update failed", head};
  }

  return {LogStoreResult::SUCCESS, "Log overwritten existing slot", head};
}

void gather_stored_logs(String &log, Preferences &preferences)
{

  for (uint8_t i = 0; i < LOG_MAX_NOTES_NUMBER; i++)
  {
    String key = PREFERENCES_LOG_KEY + String(i);
    if (preferences.isKey(key.c_str()))
    {
      String note = preferences.getString(key.c_str(), "");
      if (note.length() > 0)
      {
        log += note;

        String next_key = PREFERENCES_LOG_KEY + String(i + 1);
        if (preferences.isKey(next_key.c_str()))
        {
          log += ",";
        }
      }
    }
  }
}

void clear_stored_logs(Preferences &preferences)
{
  int count = 0;
  for (uint8_t i = 0; i < LOG_MAX_NOTES_NUMBER; i++)
  {
    String key = PREFERENCES_LOG_KEY + String(i);
    if (preferences.isKey(key.c_str()))
    {
      bool note_del = preferences.remove(key.c_str());
      if (note_del)
      {
        count++;
      }
      else
      {
        Log_error("log note not deleted");
      }
    }
  }
  Log_info("Cleared %d stored logs", count);
}