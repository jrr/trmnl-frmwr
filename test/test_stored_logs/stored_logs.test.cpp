#include <unity.h>
#include <stored_logs.h>
#include <unordered_map>
#include <string>
#include "memory_persistence.h"

MemoryPersistence persistence;

void test_stores_string(void)
{
  TEST_ASSERT_EQUAL(0, persistence.size());
  store_log("asdf", 4, persistence);
  TEST_ASSERT_EQUAL(1, persistence.size());
  String s;
  gather_stored_logs(s, persistence);
  TEST_ASSERT_EQUAL_STRING("asdf", s.c_str());
  clear_stored_logs(persistence);
  TEST_ASSERT_EQUAL(0, persistence.size());
}

void test_stores_several_strings()
{
  TEST_ASSERT_EQUAL(0, persistence.size());
  store_log("asdf", 4, persistence);
  store_log("qwer", 4, persistence);
  store_log("zxcv", 4, persistence);
  TEST_ASSERT_EQUAL(3, persistence.size());
  String s;
  gather_stored_logs(s, persistence);
  TEST_ASSERT_EQUAL_STRING("asdf,qwer,zxcv", s.c_str());
  clear_stored_logs(persistence);
  TEST_ASSERT_EQUAL(0, persistence.size());
}

void test_circular_buffer_overwrites_oldest()
{
  TEST_ASSERT_EQUAL(0, persistence.size());

  store_log("log1", 4, persistence);
  store_log("log2", 4, persistence);
  store_log("log3", 4, persistence);
  store_log("log4", 4, persistence);
  store_log("log5", 4, persistence);

  TEST_ASSERT_EQUAL(5, persistence.size());

  store_log("log6", 4, persistence);

  // Should now be 6 total (5 logs + head pointer)
  TEST_ASSERT_EQUAL(6, persistence.size());
  
  String s;
  gather_stored_logs(s, persistence);

  TEST_ASSERT(strstr(s.c_str(), "log6,") != NULL);
  TEST_ASSERT(strstr(s.c_str(), "log1,") == NULL); // log1 should be gone
  
  clear_stored_logs(persistence);

  TEST_ASSERT_EQUAL(1, persistence.size());
}

void setUp(void) {}

void tearDown(void) {}

void process()
{
  UNITY_BEGIN();
  RUN_TEST(test_stores_string);
  RUN_TEST(test_stores_several_strings);
  RUN_TEST(test_circular_buffer_overwrites_oldest);
  UNITY_END();
}

int main(int argc, char **argv)
{
  process();
  return 0;
}