/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#include "accelerator/core/apis.h"
#include "accelerator/core/mam_core.h"
#include "tests/common.h"
#include "tests/test_define.h"

static ta_core_t ta_core;
struct timespec start_time, end_time;

char driver_tag_msg[NUM_TRYTES_TAG];
ta_send_mam_res_t res;

#if defined(ENABLE_STAT)
#define TEST_COUNT 100
#else
#define TEST_COUNT 1
#endif

void test_send_mam_message(void) {
  const char* json_template = "{\"x-api-key\":\"" TEST_TOKEN "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":" STR(
      TEST_CH_DEPTH) ",\"message\":\"" TEST_PAYLOAD "\"}, \"protocol\":\"MAM_V1\"}";
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  gen_rand_trytes(NUM_TRYTES_ADDRESS, (tryte_t*)seed);
  const int len = strlen(json_template) + NUM_TRYTES_ADDRESS;
  char* json_result = NULL;
  char* json = (char*)malloc(sizeof(char) * len);
  snprintf(json, len, json_template, seed);
  double sum = 0;
  test_time_start(&start_time);
  for (size_t count = 0; count < TEST_COUNT; count++) {
    ta_send_mam_req_t* req = send_mam_req_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, send_mam_message_req_deserialize(json, req));
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, ta_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, req, &res));
    send_mam_message_res_deserialize(json_result, &res);

    free(json_result);
    send_mam_req_free(&req);
  }
  free(json);
  test_time_end(&start_time, &end_time, &sum);
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

void test_receive_mam_message(void) {
  const char* json_template = "{\"data_id\":{\"chid\":\"%s\"},\"protocol\":\"MAM_V1\"}";
  char *json = NULL, *json_result = NULL;
  double sum = 0;
  const int json_size = sizeof(char) * (strlen(json_template) + NUM_TRYTES_ADDRESS);
  json = (char*)malloc(json_size);
  snprintf(json, json_size, json_template, res.chid);
  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_recv_mam_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
  free(json);
}

void test_api_register_mam_channel(void) {
  char* json_result;
  const char* json = "{\"seed\":\"" TRYTES_81_1 "\"}";
  char user_id[UUID_STR_LEN], *seed = NULL;
  TEST_ASSERT_EQUAL_INT32(SC_OK, api_register_mam_channel(&ta_core.cache, json, &json_result));

  TEST_ASSERT_EQUAL_INT32(SC_OK, register_mam_channel_res_deserialize(json_result, user_id));
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_get(user_id, &seed));
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_del(user_id));
  free(seed);
  free(json_result);
}

int main(int argc, char* argv[]) {
  UNITY_BEGIN();
  rand_trytes_init();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }
  ta_mam_logger_init();

  ta_core_default_init(&ta_core);
  ta_core_cli_init(&ta_core, argc, argv);
  ta_core_set(&ta_core);
  ta_logger_switch(false, true, &ta_core.ta_conf);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  RUN_TEST(test_send_mam_message);
  RUN_TEST(test_receive_mam_message);
  RUN_TEST(test_api_register_mam_channel);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
