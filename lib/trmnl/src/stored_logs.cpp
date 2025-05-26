#include <trmnl_log.h>
#include <persistence_interface.h>
#include <libtrmnl_constants.h>

void store_log(const char *log_buffer, size_t size, Persistence &persistence)
{
  bool result = false;

  for (uint8_t i = 0; i < LOG_MAX_NOTES_NUMBER; i++)
  {
    String key = PREFERENCES_LOG_KEY + String(i);
    if (persistence.recordExists(key.c_str()))
    {
      Log_info("key %s exists", key.c_str());
      result = false;
    }
    else
    {
      Log_info("key %s not exists", key.c_str());
      size_t res = persistence.writeString(key.c_str(), log_buffer);
      Log_info("Initial size %d. Received size - %d", size, res);
      if (res == size)
      {
        Log_info("log note written success");
      }
      else
      {
        Log_info("log note writing failed");
      }
      result = true;
      break;
    }
  }
  if (!result)
  {
    uint8_t head = 0;
    if (persistence.recordExists(PREFERENCES_LOG_BUFFER_HEAD_KEY))
    {
      Log_info("head exists");
      head = persistence.readUChar(PREFERENCES_LOG_BUFFER_HEAD_KEY, 0);
    }
    else
    {
      Log_info("head NOT exists");
    }

    String key = PREFERENCES_LOG_KEY + String(head);
    size_t res = persistence.writeString(key.c_str(), log_buffer);
    if (res == size)
    {
      Log_info("log note written success");
    }
    else
    {
      Log_info("log note writing failed");
    }

    head += 1;
    if (head == LOG_MAX_NOTES_NUMBER)
    {
      head = 0;
    }

    uint8_t result_write = persistence.writeUChar(PREFERENCES_LOG_BUFFER_HEAD_KEY, head);
    if (result_write)
      Log_info("head written success");
    else
      Log_info("head note writing failed");
  }
}

void gather_stored_logs(String &log, Persistence &persistence)
{
  for (uint8_t i = 0; i < LOG_MAX_NOTES_NUMBER; i++)
  {
    String key = PREFERENCES_LOG_KEY + String(i);
    if (persistence.recordExists(key.c_str()))
    {
      Log_info("log note exists");
      String note = persistence.readString(key.c_str(), "");
      if (note.length() > 0)
      {
        log += note;

        String next_key = PREFERENCES_LOG_KEY + String(i + 1);
        if (persistence.recordExists(next_key.c_str()))
        {
          log += ",";
        }
      }
    }
  }
}

void clear_stored_logs(Persistence &persistence)
{

  for (uint8_t i = 0; i < LOG_MAX_NOTES_NUMBER; i++)
  {
    String key = PREFERENCES_LOG_KEY + String(i);
    if (persistence.recordExists(key.c_str()))
    {
      Log_info("log note exists");
      bool note_del = persistence.remove(key.c_str());
      if (note_del)
        Log_info("log note deleted");
      else
        Log_info("log note not deleted");
    }
  }
}