#include "test_cache.h"

void test_cache_del(void) {
  cache_t* cache = cache_init();
  const char* key = TRYTES_81_1;
  cache_del(cache, key);
  cache_stop(&cache);
}

void test_cache_get(void) {
  cache_t* cache = cache_init();
  const char* key = TRYTES_81_1;
  char res[TRYTES_2673_LEN + 1] = {0};
  cache_get(cache, key, res);
  res[TRYTES_2673_LEN] = '\0';
  TEST_ASSERT_EQUAL_STRING(res, TRYTES_2673_1);
  cache_stop(&cache);
}

void test_cache_set(void) {
  cache_t* cache = cache_init();
  const char* key = TRYTES_81_1;
  const char* value = TRYTES_2673_1;
  cache_set(cache, key, value);
  cache_stop(&cache);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_cache_set);
  RUN_TEST(test_cache_get);
  RUN_TEST(test_cache_del);
  return UNITY_END();
}
