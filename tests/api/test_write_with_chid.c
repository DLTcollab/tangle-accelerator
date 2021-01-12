/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
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

void test_write_with_chid(void) {
  const int beginning_msg_num = 1;
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  const char* json_template_send =
      "{\"x-api-key\":\"" TEST_TOKEN
      "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":1,\"message\":\"%s:%d\"}, \"protocol\":\"MAM_V1\"}";
  const char payload[] = "This is test payload number";
  const int len = strlen(json_template_send) + NUM_TRYTES_ADDRESS + strlen(payload) + 2;
  gen_rand_trytes(NUM_TRYTES_ADDRESS, (tryte_t*)seed);
  double sum = 0;
  ta_send_mam_req_t* req;
  ta_send_mam_res_t* res;
  test_time_start(&start_time);
  char* json_result = NULL;
  for (int i = 0; i < beginning_msg_num; ++i) {
    res = send_mam_res_new();
    char* json = (char*)malloc(sizeof(char) * len);
    snprintf(json, len, json_template_send, seed, payload, i);
    req = send_mam_req_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, send_mam_message_req_deserialize(json, req));
    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            ta_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, req, res));
    free(json);
    if (i != beginning_msg_num - 1) {
      free(json_result);
      send_mam_res_free(&res);
    }
    send_mam_req_free(&req);
  }
  send_mam_message_res_deserialize(json_result, res);
  free(json_result);

  // Send message from next channel ID
  const char* json_template_send_chid =
      "{\"x-api-key\":\"" TEST_TOKEN
      "\",\"data\":{\"seed\":\"%s\",\"chid\":\"%s\",\"ch_mss_depth\":2,\"message\":\"%s\"}, \"protocol\":\"MAM_V1\"}";
  const int len_send_chid = strlen(json_template_send_chid) + NUM_TRYTES_ADDRESS * 2 + strlen(payload);
  char* json_send_chid = (char*)malloc(sizeof(char) * (len_send_chid + 1));
  snprintf(json_send_chid, len_send_chid, json_template_send_chid, seed, res->chid1, payload);
  TEST_ASSERT_EQUAL_INT32(SC_OK, api_send_mam_message(&ta_core.cache, json_send_chid, &json_result));

  req = send_mam_req_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, send_mam_message_req_deserialize(json_send_chid, req));
  TEST_ASSERT_EQUAL_INT32(SC_OK,
                          ta_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, req, res));

  test_time_end(&start_time, &end_time, &sum);
  send_mam_req_free(&req);
  send_mam_res_free(&res);
  free(json_send_chid);
  free(json_result);
  printf("Elapsed time of write_with_chid: %lf\n", sum);
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
  RUN_TEST(test_write_with_chid);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
