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
  TEST_ASSERT_EQUAL_INT32(1 * 2, utarray_len(recv_res->payload_array));
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
  RUN_TEST(test_encrypt_decrypt_psk);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
