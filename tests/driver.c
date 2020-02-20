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

static struct proxy_apis_s {
  const char* name;
  const char* json;  // args which are passed to IRI API
} proxy_apis_g[] = {{"check_consistency",
                     "{\"command\":\"checkConsistency\","
                     "\"tails\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"]}"},
                    {"get_balances",
                     "{\"command\":\"getBalances\","
                     "\"addresses\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"],"
                     "\"threshold\":" STR(THRESHOLD) "}"},
                    {"get_inclusion_states",
                     "{\"command\":\"getInclusionStates\","
                     "\"transactions\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"],"
                     "\"tips\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"]}"},
                    {"get_node_info", "{\"command\": \"getNodeInfo\"}"},
                    {"get_trytes",
                     "{\"command\":\"getTrytes\","
                     "\"hashes\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"]}"}};

static const int proxy_apis_num = sizeof(proxy_apis_g) / sizeof(struct proxy_apis_s);

#if defined(ENABLE_STAT)
#define TEST_COUNT 100
#else
#define TEST_COUNT 1
#endif

#ifdef DB_ENABLE
static struct identity_s {
  char uuid_string[DB_UUID_STRING_LENGTH];
  char hash[NUM_FLEX_TRITS_HASH + 1];
  int8_t status;
} identities[TEST_COUNT];
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
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_generate_address(&ta_core.iota_conf, &ta_core.iota_service, &json_result));
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
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_get_tips_pair(&ta_core.iota_conf, &ta_core.iota_service, &json_result));
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
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_get_tips(&ta_core.iota_service, &json_result));
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
  snprintf(json, json_len + NUM_TRYTES_TAG, pre_json, driver_tag_msg);

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_send_transfer(&ta_core, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
#ifdef DB_ENABLE
    cJSON* json_obj = cJSON_Parse(json_result);
    cJSON* json_item = NULL;
    json_item = cJSON_GetObjectItemCaseSensitive(json_obj, "id");

    TEST_ASSERT(json_item != NULL && json_item->valuestring != NULL &&
                (strnlen(json_item->valuestring, DB_UUID_STRING_LENGTH - 1) == (DB_UUID_STRING_LENGTH - 1)));
    memcpy(identities[count].uuid_string, json_item->valuestring, DB_UUID_STRING_LENGTH);

    json_item = cJSON_GetObjectItemCaseSensitive(json_obj, "hash");
    TEST_ASSERT(json_item != NULL && json_item->valuestring != NULL &&
                (strnlen(json_item->valuestring, NUM_TRYTES_HASH) == NUM_TRYTES_HASH));
    memcpy(identities[count].hash, json_item->valuestring, NUM_TRYTES_HASH);
    identities[count].hash[NUM_TRYTES_HASH] = '\0';
    identities[count].status = PENDING_TXN;

    cJSON_Delete(json_obj);
#endif
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
    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_send_trytes(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of send_trytes: %lf\n", sum / TEST_COUNT);
}

void test_find_transaction_objects(void) {
  const char* json = "{\"hashes\":[\"" TRYTES_81_2 "\",\"" TRYTES_81_3 "\"]}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transaction_objects(&ta_core.iota_service, json, &json_result));
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

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_by_tag(&ta_core.iota_service, driver_tag_msg, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transactions_by_tag: %lf\n", sum / TEST_COUNT);
}

#ifdef DB_ENABLE
void test_find_transactions_by_id(void) {
  char* json_result;
  double sum = 0;
  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_by_id(&ta_core.iota_service, &ta_core.db_service,
                                                               identities[count].uuid_string, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transactions_by_id: %lf\n", sum / TEST_COUNT);
}

void test_api_get_identity_info_by_id(void) {
  char* json_result;
  double sum = 0;
  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_get_identity_info_by_id(&ta_core.db_service, identities[count].uuid_string, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_identity_info_by_id: %lf\n", sum / TEST_COUNT);
}

void test_api_get_identity_info_by_hash(void) {
  char* json_result;
  double sum = 0;
  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            api_get_identity_info_by_hash(&ta_core.db_service, identities[count].hash, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_identity_info_by_hash: %lf\n", sum / TEST_COUNT);
}
#endif

void test_find_transactions_obj_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            api_find_transactions_obj_by_tag(&ta_core.iota_service, driver_tag_msg, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_tx_obj_by_tag: %lf\n", sum / TEST_COUNT);
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
    ret = api_mam_send_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, json, &json_result);
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
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(
        SC_OK, api_receive_mam_message(&ta_core.iota_conf, &ta_core.iota_service, (char*)res->chid, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

void test_proxy_apis() {
  for (int i = 0; i < proxy_apis_num; i++) {
    char* json_result;
    double sum = 0;

    for (size_t count = 0; count < TEST_COUNT; count++) {
      test_time_start(&start_time);
      TEST_ASSERT_EQUAL_INT32(
          SC_OK, proxy_api_wrapper(&ta_core.ta_conf, &ta_core.iota_service, proxy_apis_g[i].json, &json_result));
      test_time_end(&start_time, &end_time, &sum);
      free(json_result);
    }
    printf("Average time of %s: %lf\n", proxy_apis_g[i].name, sum / TEST_COUNT);
  }
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
  RUN_TEST(test_generate_address);
  RUN_TEST(test_get_tips_pair);
  RUN_TEST(test_get_tips);
  RUN_TEST(test_send_transfer);
  RUN_TEST(test_send_trytes);
  RUN_TEST(test_find_transaction_objects);
  RUN_TEST(test_send_mam_message);
  // RUN_TEST(test_receive_mam_message);
  RUN_TEST(test_find_transactions_by_tag);
  RUN_TEST(test_find_transactions_obj_by_tag);
#ifdef DB_ENABLE
  RUN_TEST(test_api_get_identity_info_by_hash);
  RUN_TEST(test_api_get_identity_info_by_id);
  RUN_TEST(test_find_transactions_by_id);
#endif
  RUN_TEST(test_proxy_apis);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
