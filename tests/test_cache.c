/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

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
  cache_init(true, REDIS_HOST, REDIS_PORT);
  RUN_TEST(test_cache_set);
  RUN_TEST(test_cache_get);
  RUN_TEST(test_cache_del);
  cache_stop();
  return UNITY_END();
}
