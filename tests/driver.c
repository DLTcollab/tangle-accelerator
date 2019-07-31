/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <time.h>
#include "accelerator/apis.h"
#include "test_define.h"

static ta_core_t ta_core;
struct timespec start_time, end_time;

char driver_tag_msg[NUM_TRYTES_TAG];
ta_send_mam_res_t* res;

#if defined(ENABLE_STAT)
#define TEST_COUNT 100
#else
#define TEST_COUNT 1
#endif

static void gen_rand_tag(char* tag) {
  const char tryte_alpahbet[] = "NOPQRSTUVWXYZ9ABCDEFGHIJKLM";

  for (int i = 0; i < NUM_TRYTES_TAG; i++) {
    tag[i] = tryte_alpahbet[rand() % 27];
  }
}

double diff_time(struct timespec start, struct timespec end) {
  struct timespec diff;
  if (end.tv_nsec - start.tv_nsec < 0) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec + 1000000000;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

void test_time_start(struct timespec* start) { clock_gettime(CLOCK_REALTIME, start); }

void test_time_end(struct timespec* start, struct timespec* end, double* sum) {
  clock_gettime(CLOCK_REALTIME, end);
  double difference = diff_time(*start, *end);
#if defined(ENABLE_STAT)
  printf("%lf\n", difference);
#endif
  *sum += difference;
}

void test_generate_address(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_generate_address(&ta_core.tangle, &ta_core.service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of generate_address: %lf\n", sum / TEST_COUNT);
}

void test_get_tips_pair(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_get_tips_pair(&ta_core.tangle, &ta_core.service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_tips_pair: %lf\n", sum / TEST_COUNT);
}

void test_get_tips(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_get_tips(&ta_core.service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_tips: %lf\n", sum / TEST_COUNT);
}

void test_send_transfer(void) {
  const char* pre_json =
      "{\"value\":100,"
      "\"message\":\"" TAG_MSG
      "\",\"tag\":\"%s\","
      "\"address\":\"" TRYTES_81_1 "\"}";
  char* json_result;
  double sum = 0;

  gen_rand_tag(driver_tag_msg);
  int json_len = strlen(pre_json);
  char json[json_len + NUM_TRYTES_TAG];
  sprintf(json, pre_json, driver_tag_msg);

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_send_transfer(&ta_core.tangle, &ta_core.service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of send_transfer: %lf\n", sum / TEST_COUNT);
}

void test_send_trytes(void) {
  const char* json = "{\"trytes\":[\"" TRYTES_2673_1 "\",\"" TRYTES_2673_2 "\"]}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_send_trytes(&ta_core.tangle, &ta_core.service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of send_trytes: %lf\n", sum / TEST_COUNT);
}

void test_find_transactions(void) {
  const char* json = "{\"addresses\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"]}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions(&ta_core.service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transactions: %lf\n", sum / TEST_COUNT);
}

void test_find_transaction_objects(void) {
  const char* json = "{\"hashes\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"]}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transaction_objects(&ta_core.service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transaction_objects: %lf\n", sum / TEST_COUNT);
}

void test_find_transactions_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_by_tag(&ta_core.service, driver_tag_msg, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transactions_by_tag: %lf\n", sum / TEST_COUNT);
}

void test_find_transactions_obj_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_obj_by_tag(&ta_core.service, driver_tag_msg, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_tx_obj_by_tag: %lf\n", sum / TEST_COUNT);
}

void test_send_mam_message(void) {
  double sum = 0;
  const char* json = "{\"message\":\"" TEST_PAYLOAD "\"}";
  char* json_result;
  res = send_mam_res_new();

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_mam_send_message(&ta_core.tangle, &ta_core.service, json, &json_result));
    send_mam_res_deserialize(json_result, res);

    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of send_mam_message: %lf\n", sum / TEST_COUNT);
}

void test_receive_mam_message(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_receive_mam_message(&ta_core.service, (char*)res->channel_id, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

int main(void) {
  srand(time(NULL));

  UNITY_BEGIN();

  ta_config_default_init(&ta_core.info, &ta_core.tangle, &ta_core.cache, &ta_core.service);
  ta_config_set(&ta_core.cache, &ta_core.service);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  RUN_TEST(test_generate_address);
  RUN_TEST(test_get_tips_pair);
  RUN_TEST(test_get_tips);
  RUN_TEST(test_send_transfer);
  RUN_TEST(test_send_trytes);
  RUN_TEST(test_find_transaction_objects);
  RUN_TEST(test_find_transactions);
  RUN_TEST(test_send_mam_message);
  RUN_TEST(test_receive_mam_message);
  RUN_TEST(test_find_transactions_by_tag);
  RUN_TEST(test_find_transactions_obj_by_tag);
  ta_config_destroy(&ta_core.service);
  return UNITY_END();
}
