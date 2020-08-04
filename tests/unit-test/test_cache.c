/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <pthread.h>
#include "tests/test_define.h"
#include "utils/cache/cache.h"
#include "uuid/uuid.h"

char test_uuid[UUID_STR_LEN] = {};

void test_cache_del(void) {
  char* key = test_uuid;
  TEST_ASSERT_EQUAL_INT(SC_OK, cache_del(key));
}

void test_cache_get(void) {
  char* key = test_uuid;
  char* res = NULL;

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_get(key, &res));
  TEST_ASSERT_EQUAL_STRING(CACHE_VALUE, res);
  free(res);
}

void test_cache_set(void) {
  char* key = test_uuid;

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_set(key, strlen(key), CACHE_VALUE, strlen(CACHE_VALUE), 0));
}

void test_cache_timeout(void) {
  char* key = test_uuid;
  char* res = NULL;
  const int timeout = 2;
  TEST_ASSERT_EQUAL_INT(SC_OK, cache_set(key, strlen(key), CACHE_VALUE, strlen(CACHE_VALUE), timeout));
  TEST_ASSERT_EQUAL_INT(SC_OK, cache_get(key, &res));
  free(res);
  res = NULL;
  sleep(timeout + 1);
  TEST_ASSERT_EQUAL_INT(SC_CACHE_FAILED_RESPONSE, cache_get(key, &res));
  free(res);
}

void test_generate_uuid(void) {
  uuid_t binuuid;
  uuid_generate_random(binuuid);
  uuid_unparse(binuuid, test_uuid);

  TEST_ASSERT_TRUE(test_uuid[0]);
}

void test_cache_list_push(void) {
  TEST_ASSERT_EQUAL_INT(
      SC_OK, cache_list_push(TEST_UUID_LIST_NAME, strlen(TEST_UUID_LIST_NAME), TEST_UUID, strlen(TEST_UUID)));
}

void test_cache_list_at(void) {
  char res[UUID_STR_LEN];

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_list_at(TEST_UUID_LIST_NAME, -1, UUID_STR_LEN, res));
  TEST_ASSERT_EQUAL_STRING(TEST_UUID, res);
}

void test_cache_list_size(void) {
  int len = 0;

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_list_size(TEST_UUID_LIST_NAME, &len));
  TEST_ASSERT_EQUAL_INT(1, len);
}

void test_cache_list_pop(void) {
  char res[UUID_STR_LEN];

  TEST_ASSERT_EQUAL_INT(SC_OK, cache_list_pop(TEST_UUID_LIST_NAME, res));
  TEST_ASSERT_EQUAL_STRING(TEST_UUID, res);
}

void test_cache_occupied_space() { TEST_ASSERT_GREATER_THAN(-1, cache_occupied_space()); }

int main(void) {
  UNITY_BEGIN();
  pthread_rwlock_t* rwlock = NULL;
  cache_init(&rwlock, true, REDIS_HOST, REDIS_PORT);
  RUN_TEST(test_generate_uuid);
  RUN_TEST(test_cache_set);
  RUN_TEST(test_cache_get);
  RUN_TEST(test_cache_del);
  RUN_TEST(test_cache_timeout);
  RUN_TEST(test_cache_list_push);
  RUN_TEST(test_cache_list_at);
  RUN_TEST(test_cache_list_size);
  RUN_TEST(test_cache_list_pop);
  RUN_TEST(test_cache_occupied_space);
  cache_stop(&rwlock);
  return UNITY_END();
}
