#include "test_define.h"
#include "utils/cache.h"

void test_cache_del(void) {
  const char* key = TRYTES_81_1;
  cache_del(key);
}

void test_cache_get(void) {
  const char* key = TRYTES_81_1;
  char res[TRYTES_2673_LEN + 1] = {0};
  cache_get(key, res);
  res[TRYTES_2673_LEN] = '\0';
  TEST_ASSERT_EQUAL_STRING(res, TRYTES_2673_1);
}

void test_cache_set(void) {
  const char* key = TRYTES_81_1;
  const char* value = TRYTES_2673_1;
  cache_set(key, value);
}

int main(void) {
  UNITY_BEGIN();
  cache_init();
  RUN_TEST(test_cache_set);
  RUN_TEST(test_cache_get);
  RUN_TEST(test_cache_del);
  cache_stop();
  return UNITY_END();
}
