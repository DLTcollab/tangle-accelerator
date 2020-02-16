/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "tests/test_define.h"
#include "utils/cache/cache.h"

void test_cache_del(void) {
  const char* key = TEST_UUID;
  TEST_ASSERT_EQUAL_INT(SC_OK, cache_del(key));
}

void test_cache_get(void) {
  const char* key = TEST_UUID;
  char res[strlen(CACHE_VALUE) + 1];

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_get(key, res));
  TEST_ASSERT_EQUAL_STRING(CACHE_VALUE, res);
}

void test_cache_set(void) {
  char key[] = TEST_UUID;
  const char* value = CACHE_VALUE;

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_set(key, strlen(key), value, strlen(value), 0));
}

void test_cache_timeout(void) {
  char key[] = TEST_UUID;
  const char* value = CACHE_VALUE;
  char res[strlen(CACHE_VALUE) + 1];
  const int timeout = 2;
  TEST_ASSERT_EQUAL_INT(SC_OK, cache_set(key, strlen(key), value, strlen(value), timeout));
  TEST_ASSERT_EQUAL_INT(SC_OK, cache_get(key, res));
  sleep(timeout + 1);
  TEST_ASSERT_EQUAL_INT(SC_CACHE_FAILED_RESPONSE, cache_get(key, res));
}

int main(void) {
  UNITY_BEGIN();
  cache_init(true, REDIS_HOST, REDIS_PORT);
  RUN_TEST(test_cache_set);
  RUN_TEST(test_cache_get);
  RUN_TEST(test_cache_del);
  RUN_TEST(test_cache_timeout);
  cache_stop();
  return UNITY_END();
}
