/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <time.h>
#include "accelerator/core/apis.h"
#include "accelerator/core/proxy_apis.h"
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

void test_send_mam_message(void) {
  double sum = 0;
  const char* json =
      "{\"seed\":\"" TRYTES_81_1
      "\",\"channel_ord\":" STR(TEST_CHANNEL_ORD) ",\"message\":\"" TEST_PAYLOAD "\",\"ch_mss_depth\":" STR(
          TEST_CH_DEPTH) ",\"ep_mss_depth\":" STR(TEST_EP_DEPTH) ",\"ntru_pk\":\"" TEST_NTRU_PK
                                                                 "\",\"psk\":\"" TRYTES_81_2 "\"}";
  char* json_result = NULL;
  res = send_mam_res_new();
  status_t ret = SC_OK;
  bool result = false;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    ret = api_mam_send_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result);
    if (ret == SC_OK || ret == SC_MAM_ALL_MSS_KEYS_USED) {
      result = true;
    }
    TEST_ASSERT_TRUE(result);

    send_mam_res_deserialize(json_result, res);

    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
    json_result = NULL;
  }
  free(json_result);
  printf("Average time of send_mam_message: %lf\n", sum / TEST_COUNT);
}

void test_receive_mam_message(void) {
  const char* json =
      "{\"data_id\":{\"chid\":\"SYZJOQCGWGOTGGZUCZYQGHWWHQVIVBHJFHNXYXZQSGJXIHL9KMXNOBULIPTPGMBJSMFNBSAGCVVZ9MDMX\"},"
      "\"protocol\":\"MAM_V1\"}";
  char* json_result = NULL;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            api_receive_mam_message(&ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

int main(int argc, char* argv[]) {
  srand(time(NULL));

  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  ta_core_default_init(&ta_core);
  ta_core_cli_init(&ta_core, argc, argv);
  ta_core_set(&ta_core);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  // RUN_TEST(test_send_mam_message);
  RUN_TEST(test_receive_mam_message);

  ta_core_destroy(&ta_core);
  return UNITY_END();
}
