#include <unity.h>
#include <stored_logs.h>
#include <unordered_map>
#include <string>
#include "memory_persistence.h"

void test_stores_string(void)
{
  // Test basic storage with (0,3) configuration
  MemoryPersistence persistence;
  StoredLogs subject(0, 3, "log_", "log_head", persistence);
  
  subject.store_log("asdf");
  TEST_ASSERT_EQUAL_STRING("asdf", subject.gather_stored_logs().c_str());
}

void test_stores_several_strings()
{
  // Test storing multiple strings with (0,3) - fills all 3 newest slots
  MemoryPersistence persistence;
  StoredLogs subject(0, 3, "log_", "log_head", persistence);
  
  subject.store_log("asdf");
  subject.store_log("qwer");
  subject.store_log("zxcv");
  TEST_ASSERT_EQUAL_STRING("asdf,qwer,zxcv", subject.gather_stored_logs().c_str());
}

void test_circular_buffer_overwrites_oldest()
{
  // Test (0,3) circular buffer behavior - overwrites when exceeding 3 newest slots
  MemoryPersistence persistence;
  StoredLogs subject(0, 3, "log_", "log_head", persistence);
  
  subject.store_log("log1");
  subject.store_log("log2");
  subject.store_log("log3");
  TEST_ASSERT_EQUAL_STRING("log1,log2,log3", subject.gather_stored_logs().c_str());
  
  // Adding 4th item should overwrite log1, chronological order: log2,log3,log4
  subject.store_log("log4");
  TEST_ASSERT_EQUAL_STRING("log2,log3,log4", subject.gather_stored_logs().c_str());
}

void test_overwrite_counter()
{
  // Test overwrite counter with (0,3) - counts when circular buffer wraps
  MemoryPersistence persistence;
  StoredLogs subject(0, 3, "log_", "log_head", persistence);
  
  TEST_ASSERT_EQUAL(0, subject.get_overwrite_count());

  // Fill all slots - no overwrites yet
  subject.store_log("log1");
  subject.store_log("log2");
  subject.store_log("log3");
  TEST_ASSERT_EQUAL(0, subject.get_overwrite_count());

  // Start overwriting
  subject.store_log("log4");
  TEST_ASSERT_EQUAL(1, subject.get_overwrite_count());

  subject.store_log("log5");
  TEST_ASSERT_EQUAL(2, subject.get_overwrite_count());

  // Clear should reset counter
  subject.clear_stored_logs();
  TEST_ASSERT_EQUAL(0, subject.get_overwrite_count());
}

void test_keeps_oldest_only()
{
  // Test (3,0) configuration - keeps only 3 oldest items
  MemoryPersistence persistence;
  StoredLogs subject(3, 0, "old_", "old_head", persistence);
  
  subject.store_log("first");
  subject.store_log("second");
  subject.store_log("third");
  TEST_ASSERT_EQUAL_STRING("first,second,third", subject.gather_stored_logs().c_str());
  
  // Adding 4th item should discard it, keeping only the first 3
  subject.store_log("fourth");
  TEST_ASSERT_EQUAL_STRING("first,second,third", subject.gather_stored_logs().c_str());
  
  // Adding 5th item should also be discarded
  subject.store_log("fifth");
  TEST_ASSERT_EQUAL_STRING("first,second,third", subject.gather_stored_logs().c_str());
}

void test_mixed_mode_1_oldest_2_newest()
{
  // Test (1,2) configuration - keeps 1 oldest, 2 newest
  MemoryPersistence persistence;
  StoredLogs subject(1, 2, "mix_", "mix_head", persistence);
  
  // Fill up: 1 oldest + 2 newest
  subject.store_log("first");   // goes to oldest slot 0
  subject.store_log("second");  // goes to newest slot 1
  subject.store_log("third");   // goes to newest slot 2
  TEST_ASSERT_EQUAL_STRING("first,second,third", subject.gather_stored_logs().c_str());
  
  // Add 4th item - should overwrite "second" in newest section
  subject.store_log("fourth");
  TEST_ASSERT_EQUAL_STRING("first,third,fourth", subject.gather_stored_logs().c_str());
  
  // Add 5th item - should overwrite "third" in newest section  
  subject.store_log("fifth");
  TEST_ASSERT_EQUAL_STRING("first,fourth,fifth", subject.gather_stored_logs().c_str());
}

void setUp(void) {}

void tearDown(void) {}

void process()
{
  UNITY_BEGIN();
  RUN_TEST(test_stores_string);
  RUN_TEST(test_stores_several_strings);
  RUN_TEST(test_circular_buffer_overwrites_oldest);
  RUN_TEST(test_overwrite_counter);
  RUN_TEST(test_keeps_oldest_only);
  RUN_TEST(test_mixed_mode_1_oldest_2_newest);
  UNITY_END();
}

int main(int argc, char **argv)
{
  process();
  return 0;
}