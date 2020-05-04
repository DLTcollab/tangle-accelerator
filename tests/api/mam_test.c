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
#include "common.h"
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

// FIXME Use a common "gen_rand_seed()" function to replace a static function
static void rand_seed_init() {
  // We use ASLR which stands for Address Space Layout Randomization, and we can assume each address of functions inside
  // loaded program is randomized.
  srand(getpid() ^ ((int)&rand_seed_init));
}

static void gen_rand_seed(char* trytes) {
  const char tryte_alphabet[] = "NOPQRSTUVWXYZ9ABCDEFGHIJKLM";

  for (int i = 0; i < NUM_TRYTES_HASH; i++) {
    trytes[i] = tryte_alphabet[rand() % 27];
  }
}

void test_send_mam_message(void) {
  const char* json_template = "{\"x-api-key\":\"" TEST_TOKEN "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":" STR(
      TEST_CH_DEPTH) ",\"message\":\"" TEST_PAYLOAD "\"}, \"protocol\":\"MAM_V1\"}";
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  gen_rand_seed(seed);
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

void test_send_read_mam_bundle(void) {
  mam_api_t mam;
  tryte_t chid[MAM_CHANNEL_ID_TRYTE_SIZE] = {}, msg_id[NUM_TRYTES_MAM_MSG_ID] = {};
  mam_psk_t_set_t psks = NULL;
  mam_ntru_pk_t_set_t ntru_pks = NULL;

  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);

  ta_send_mam_req_t* req = send_mam_req_new();
  const char* json_template = "{\"x-api-key\":\"" TEST_TOKEN "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":" STR(
      TEST_CH_DEPTH) ",\"message\":\"" TEST_PAYLOAD "\"}, \"protocol\":\"MAM_V1\"}";
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  gen_rand_seed(seed);
  const int len = strlen(json_template) + NUM_TRYTES_ADDRESS;
  char* json_result = NULL;
  char* json = (char*)malloc(sizeof(char) * len);
  snprintf(json, len, json_template, seed);
  TEST_ASSERT_EQUAL_INT32(SC_OK, send_mam_req_deserialize(json, req));

  send_mam_data_mam_v1_t* data = (send_mam_data_mam_v1_t*)req->data;

  // Creating MAM API
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_mam_init(&mam, &ta_core.iota_conf, data->seed));
  mam_mss_key_status_t key_status;
  // Create epid merkle tree and find the smallest unused secret key.
  // Write both Header and Pakcet into one single bundle.
  TEST_ASSERT_EQUAL_INT32(
      SC_OK, ta_mam_written_msg_to_bundle(&ta_core.iota_service, &mam, data->ch_mss_depth, chid, psks, ntru_pks,
                                          data->message, &bundle, msg_id, &key_status));

  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_mam_api_bundle_read(&mam, bundle, &json_result));

  TEST_ASSERT_EQUAL_STRING(TEST_PAYLOAD, json_result);

  free(json_result);
  send_mam_req_free(&req);
  bundle_transactions_free(&bundle);
  TEST_ASSERT_EQUAL_INT32(RC_OK, mam_api_destroy(&mam));
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
  gen_rand_seed(seed);
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
  for (int i = 0; i < complete_ch_num + 1; i++) {
    char *json = NULL, *json_result = NULL;
    json = (char*)malloc(json_size);
    snprintf(json, json_size, json_template_recv, mam_res_array[i * channel_leaf_msg_num]->chid);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_recv_mam_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    for (int j = i * channel_leaf_msg_num; j < ((i + 1) * channel_leaf_msg_num) && (j < msg_num); j++) {
      // Check whether a message exist under assigning channel ID.
      char substr[strlen(payload) + 3];
      snprintf(substr, strlen(payload) + 3, "%s:%d", payload, j);
      TEST_ASSERT_TRUE(strstr(json_result, substr));
    }

    // Deserialize the response, and compare whether the payload list contains correct number of message under a certain
    // channel ID.
    int len = 0;
    cJSON *json_obj = cJSON_Parse(json_result), *current_obj = NULL;
    cJSON* json_item = cJSON_GetObjectItemCaseSensitive(json_obj, "payload");
    TEST_ASSERT_TRUE(cJSON_IsArray(json_item));
    cJSON_ArrayForEach(current_obj, json_item) {
      if (current_obj->valuestring != NULL) {
        len++;
      }
    }
    if (i == complete_ch_num) {
      TEST_ASSERT_EQUAL_INT16(dangling_msg_num, len);
    } else {
      TEST_ASSERT_EQUAL_INT16(channel_leaf_msg_num, len);
    }
    cJSON_Delete(json_obj);
    free(json);
    free(json_result);
  }

  for (int i = 0; i < msg_num; i++) {
    send_mam_res_free(&(mam_res_array[i]));
  }
}

int main(int argc, char* argv[]) {
  UNITY_BEGIN();
  rand_seed_init();

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
  RUN_TEST(test_send_read_mam_bundle);
  RUN_TEST(test_write_until_next_channel);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
