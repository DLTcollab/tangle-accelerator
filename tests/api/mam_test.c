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
  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    send_mam_res_deserialize(json_result, &res);
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  free(json);
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

void test_write_until_next_channel(void) {
  const int channel_leaf_msg_num = (1 << TEST_CH_DEPTH) - 1;
  const int complete_ch_num = 1, dangling_msg_num = 1;
  const int msg_num = complete_ch_num * channel_leaf_msg_num + dangling_msg_num;
  ta_send_mam_res_t* mam_res_array[msg_num];
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  const char* json_template_send = "{\"x-api-key\":\"" TEST_TOKEN "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":" STR(
      TEST_CH_DEPTH) ",\"message\":\"%s:%d\"}, \"protocol\":\"MAM_V1\"}";
  const char payload[] = "This is test payload number";
  const int len = strlen(json_template_send) + NUM_TRYTES_ADDRESS + strlen(payload) + 2;
  gen_rand_trytes(NUM_TRYTES_ADDRESS, (tryte_t*)seed);
  for (int i = 0; i < msg_num; i++) {
    char* json_result;
    mam_res_array[i] = send_mam_res_new();
    char* json = (char*)malloc(sizeof(char) * len);
    snprintf(json, len, json_template_send, seed, payload, i);
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    send_mam_res_deserialize(json_result, mam_res_array[i]);
    free(json);
    free(json_result);
  }

  // The current chid1 should be equal to the chid of next channel. Element with index `channel_leaf_msg_num` is the
  // last element of the current channel.
  TEST_ASSERT_EQUAL_STRING(mam_res_array[channel_leaf_msg_num]->chid, mam_res_array[channel_leaf_msg_num - 1]->chid1);

  const char* json_template_recv = "{\"data_id\":{\"chid\":\"%s\"},\"protocol\":\"MAM_V1\"}";
  const int json_size = sizeof(char) * (strlen(json_template_recv) + NUM_TRYTES_ADDRESS);
  char chid1[NUM_TRYTES_ADDRESS + 1] = {};
  for (int i = 0; i < complete_ch_num + 1; i++) {
    char *json = NULL, *json_result = NULL;
    ta_recv_mam_res_t* res = recv_mam_res_new();
    TEST_ASSERT_NOT_NULL(res);
    json = (char*)malloc(json_size);
    snprintf(json, json_size, json_template_recv, mam_res_array[i * channel_leaf_msg_num]->chid);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_recv_mam_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    for (int j = i * channel_leaf_msg_num; j < ((i + 1) * channel_leaf_msg_num) && (j < msg_num); j++) {
      // Check whether a message exist under assigning channel ID.
      char substr[strlen(payload) + 3];
      snprintf(substr, strlen(payload) + 3, "%s:%d", payload, j);
      TEST_ASSERT_TRUE(strstr(json_result, substr));
    }

    recv_mam_res_deserialize(json_result, res);
    if (res->chid1[0]) {
      strncpy(chid1, res->chid1, NUM_TRYTES_ADDRESS);
    } else if (chid1[0]) {
      TEST_ASSERT_EQUAL_STRING(chid1, mam_res_array[i * channel_leaf_msg_num]->chid);
    }

    // Compare whether the payload list contains correct number of message under a certain channel ID.
    if (i == complete_ch_num) {
      TEST_ASSERT_EQUAL_INT16(dangling_msg_num, utarray_len(res->payload_array));
    } else {
      TEST_ASSERT_EQUAL_INT16(channel_leaf_msg_num, utarray_len(res->payload_array));
    }
    free(json);
    free(json_result);
    recv_mam_res_free(&res);
  }

  for (int i = 0; i < msg_num; i++) {
    send_mam_res_free(&(mam_res_array[i]));
  }
}

int main(int argc, char* argv[]) {
  UNITY_BEGIN();
  rand_trytes_init();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }
  apis_logger_init();
  ta_mam_logger_init();
  cc_logger_init();
  pow_logger_init();
  timer_logger_init();

  ta_core_default_init(&ta_core);
  ta_core_cli_init(&ta_core, argc, argv);
  ta_core_set(&ta_core);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  RUN_TEST(test_send_mam_message);
  RUN_TEST(test_receive_mam_message);
  RUN_TEST(test_write_until_next_channel);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
