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
  double sum = 0;
  test_time_start(&start_time);
  for (int i = 0; i < msg_num; i++) {
    mam_res_array[i] = send_mam_res_new();
    char* json = (char*)malloc(sizeof(char) * len);
    snprintf(json, len, json_template_send, seed, payload, i);

    ta_send_mam_req_t* req = send_mam_req_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, send_mam_message_req_deserialize(json, req));
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, ta_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, req, mam_res_array[i]));
    send_mam_req_free(&req);
    free(json);
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
    test_time_end(&start_time, &end_time, &sum);

    recv_mam_message_res_deserialize(json_result, res);
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

  printf("Average time of write_until_next_channel: %lf\n", sum / TEST_COUNT);
  for (int i = 0; i < msg_num; i++) {
    send_mam_res_free(&(mam_res_array[i]));
  }
}

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

void test_encrypt_decrypt_psk(void) {
  const char* json_template_send =
      "{\"x-api-key\":\"" TEST_TOKEN
      "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":" STR(TEST_CH_DEPTH) ",\"message\":\"" TEST_PAYLOAD
                                                                          "\"},\"key\":{\"psk\":[\"" TEST_PSK
                                                                          "\"]}, \"protocol\":\"MAM_V1\"}";
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  gen_rand_trytes(NUM_TRYTES_ADDRESS, (tryte_t*)seed);
  const size_t len_send = strlen(json_template_send) + NUM_TRYTES_ADDRESS;
  char* json_result = NULL;
  char* json = (char*)malloc(sizeof(char) * len_send);
  snprintf(json, len_send, json_template_send, seed);
  ta_send_mam_res_t* send_res = send_mam_res_new();
  ta_send_mam_req_t* req = send_mam_req_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, send_mam_message_req_deserialize(json, req));
  TEST_ASSERT_EQUAL_INT32(
      SC_OK, ta_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, req, send_res));
  send_mam_req_free(&req);
  free(json_result);
  free(json);

  const char* json_template_recv_psk =
      "{\"data_id\":{\"chid\":\"%s\"},\"key\":{\"psk\":[\"" TEST_PSK "\"]},\"protocol\":\"MAM_V1\"}";
  const size_t len_recv_psk = strlen(json_template_recv_psk) + NUM_TRYTES_ADDRESS;
  json = (char*)malloc(sizeof(char) * len_recv_psk);
  snprintf(json, len_recv_psk, json_template_recv_psk, send_res->chid);
  TEST_ASSERT_EQUAL_INT32(SC_OK, api_recv_mam_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result));

  // Deserialize json_result, compare the number which is equal to the number of sent message
  ta_recv_mam_res_t* recv_res = recv_mam_res_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, recv_mam_message_res_deserialize(json_result, recv_res));
  TEST_ASSERT_EQUAL_INT32(1, utarray_len(recv_res->payload_array));
  recv_mam_res_free(&recv_res);
  free(json_result);
  free(json);

  // Receive the message once again, but this time we don't add PSK key
  const char* json_template_recv = "{\"data_id\":{\"chid\":\"%s\"},\"protocol\":\"MAM_V1\"}";
  const size_t len_recv = strlen(json_template_recv) + NUM_TRYTES_ADDRESS;
  json = (char*)malloc(sizeof(char) * len_recv);
  snprintf(json, len_recv, json_template_recv, send_res->chid);
  TEST_ASSERT_EQUAL_INT32(SC_OK, api_recv_mam_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
  recv_res = recv_mam_res_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, recv_mam_message_res_deserialize(json_result, recv_res));
  TEST_ASSERT_EQUAL_INT32(0, utarray_len(recv_res->payload_array));
  recv_mam_res_free(&recv_res);
  free(json_result);
  free(json);

  send_mam_res_free(&send_res);
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
  RUN_TEST(test_write_until_next_channel);
  RUN_TEST(test_write_with_chid);
  RUN_TEST(test_encrypt_decrypt_psk);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
