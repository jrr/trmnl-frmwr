#include <unity.h>
#include <stored_logs.h>
#include <unordered_map>
#include <string>
#include "memory_persistence.h"

MemoryPersistence persistence;
// Test configuration: 0 oldest + 3 newest = current circular buffer behavior
StoredLogs subject(0, 3, "log_", "log_head", persistence);

void test_stores_string(void)
{
  // Test basic storage with (0,3) configuration
  subject.store_log("asdf");
  TEST_ASSERT_EQUAL_STRING("asdf", subject.gather_stored_logs().c_str());
  subject.clear_stored_logs();
}

void test_stores_several_strings()
{
  // Test storing multiple strings with (0,3) - fills all 3 newest slots
  subject.store_log("asdf");
  subject.store_log("qwer");
  subject.store_log("zxcv");
  TEST_ASSERT_EQUAL_STRING("asdf,qwer,zxcv", subject.gather_stored_logs().c_str());
  subject.clear_stored_logs();
}

void test_circular_buffer_overwrites_oldest()
{
  // Test (0,3) circular buffer behavior - overwrites when exceeding 3 newest slots
  subject.store_log("log1");
  subject.store_log("log2");
  subject.store_log("log3");
  TEST_ASSERT_EQUAL_STRING("log1,log2,log3", subject.gather_stored_logs().c_str());
  
  // Adding 4th item should overwrite log1 in slot 0
  subject.store_log("log4");
  TEST_ASSERT_EQUAL_STRING("log4,log2,log3", subject.gather_stored_logs().c_str());
  
  subject.clear_stored_logs();
}

void test_overwrite_counter()
{
  // Test overwrite counter with (0,3) - counts when circular buffer wraps
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

void setUp(void) {}

void tearDown(void) {}

void process()
{
  UNITY_BEGIN();
  RUN_TEST(test_stores_string);
  RUN_TEST(test_stores_several_strings);
  RUN_TEST(test_circular_buffer_overwrites_oldest);
  RUN_TEST(test_overwrite_counter);
  UNITY_END();
}

int main(int argc, char **argv)
{
  process();
  return 0;
}