#include <stored_logs.h>
#include <trmnl_log.h>
#include <persistence_interface.h>

StoredLogs::StoredLogs(uint8_t old_count, uint8_t new_count, const char *log_key, const char *head_key, Persistence& persistence)
    : old_count(old_count), new_count(new_count), log_key(log_key), head_key(head_key), persistence(persistence), overwrite_count(0) {}

LogStoreResult StoredLogs::store_log(const String& log_buffer)
{
  uint8_t max_notes = old_count + new_count;
  
  // Special case: if new_count is 0, only keep oldest items
  if (new_count == 0)
  {
    // Find first empty slot in oldest section
    for (uint8_t i = 0; i < old_count; i++)
    {
      String key = log_key + String(i);
      if (!persistence.recordExists(key.c_str()))
      {
        size_t res = persistence.writeString(key.c_str(), log_buffer.c_str());
        if (res == log_buffer.length())
        {
          return {LogStoreResult::SUCCESS, "Log stored in oldest slot", i};
        }
        else
        {
          return {LogStoreResult::FAILURE, "Failed to write to oldest slot", i};
        }
      }
    }
    // All oldest slots full, discard new log
    return {LogStoreResult::SUCCESS, "Log discarded - oldest slots full", 0};
  }
  
  // Special case: if old_count is 0, use pure circular buffer (existing behavior)
  if (old_count == 0)
  {
    // Try to find an empty slot first
    for (uint8_t i = 0; i < new_count; i++)
    {
      String key = log_key + String(i);
      if (!persistence.recordExists(key.c_str()))
      {
        size_t res = persistence.writeString(key.c_str(), log_buffer.c_str());
        if (res == log_buffer.length())
        {
          return {LogStoreResult::SUCCESS, "Log stored in new slot", i};
        }
        else
        {
          return {LogStoreResult::FAILURE, "Failed to write to new slot", i};
        }
      }
    }

    // All slots full, use circular buffer behavior
    uint8_t head = persistence.readUChar(head_key, 0);
    
    String key = log_key + String(head);
    size_t res = persistence.writeString(key.c_str(), log_buffer.c_str());
    if (res != log_buffer.length())
    {
      return {LogStoreResult::FAILURE, "Failed to overwrite slot", head};
    }

    uint8_t next_head = (head + 1) % new_count;
    if (!persistence.writeUChar(head_key, next_head))
    {
      return {LogStoreResult::FAILURE, "Log written but head update failed", head};
    }

    overwrite_count++;
    return {LogStoreResult::SUCCESS, "Log overwrote existing slot", head};
  }
  
  // Mixed mode: both old_count > 0 and new_count > 0
  // First fill oldest slots, then use circular buffer for newest slots
  for (uint8_t i = 0; i < old_count; i++)
  {
    String key = log_key + String(i);
    if (!persistence.recordExists(key.c_str()))
    {
      size_t res = persistence.writeString(key.c_str(), log_buffer.c_str());
      if (res == log_buffer.length())
      {
        return {LogStoreResult::SUCCESS, "Log stored in oldest slot", i};
      }
      else
      {
        return {LogStoreResult::FAILURE, "Failed to write to oldest slot", i};
      }
    }
  }
  
  // Oldest slots full, now use circular buffer for newest slots
  uint8_t head = persistence.readUChar(head_key, old_count); // Start head at old_count
  
  String key = log_key + String(head);
  size_t res = persistence.writeString(key.c_str(), log_buffer.c_str());
  if (res != log_buffer.length())
  {
    return {LogStoreResult::FAILURE, "Failed to overwrite newest slot", head};
  }

  uint8_t next_head = old_count + ((head - old_count + 1) % new_count);
  if (!persistence.writeUChar(head_key, next_head))
  {
    return {LogStoreResult::FAILURE, "Log written but head update failed", head};
  }

  overwrite_count++;
  return {LogStoreResult::SUCCESS, "Log overwrote newest slot", head};
}

String StoredLogs::gather_stored_logs()
{
  String log;
  uint8_t max_notes = old_count + new_count;
  
  // Count how many slots are actually filled
  uint8_t filled_slots = 0;
  for (uint8_t i = 0; i < max_notes; i++)
  {
    String key = log_key + String(i);
    if (persistence.recordExists(key.c_str()))
    {
      filled_slots++;
    }
  }
  
  if (filled_slots == 0) return log;
  
  // If buffer is not full, use simple order
  if (filled_slots < max_notes)
  {
    for (uint8_t i = 0; i < max_notes; i++)
    {
      String key = log_key + String(i);
      if (persistence.recordExists(key.c_str()))
      {
        String note = persistence.readString(key.c_str(), "");
        if (note.length() > 0)
        {
          if (log.length() > 0) log += ",";
          log += note;
        }
      }
    }
  }
  else
  {
    // Buffer is full, use head pointer to determine chronological order
    uint8_t head = persistence.readUChar(head_key, 0);
    
    // Start from head position (oldest) and wrap around
    for (uint8_t i = 0; i < max_notes; i++)
    {
      uint8_t slot = (head + i) % max_notes;
      String key = log_key + String(slot);
      if (persistence.recordExists(key.c_str()))
      {
        String note = persistence.readString(key.c_str(), "");
        if (note.length() > 0)
        {
          if (log.length() > 0) log += ",";
          log += note;
        }
      }
    }
  }
  
  return log;
}

void StoredLogs::clear_stored_logs()
{
  int count = 0;
  uint8_t max_notes = old_count + new_count;
  for (uint8_t i = 0; i < max_notes; i++)
  {
    String key = log_key + String(i);
    if (persistence.recordExists(key.c_str()))
    {
      bool note_del = persistence.remove(key.c_str());
      if (note_del)
        count++;
    }
  }
  overwrite_count = 0;
  Log_info("Cleared %d stored logs", count);
}

uint32_t StoredLogs::get_overwrite_count()
{
  return overwrite_count;
}