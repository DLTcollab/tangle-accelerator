/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <time.h>
#include "accelerator/core/apis.h"
#include "accelerator/core/proxy_apis.h"
#include "cJSON.h"
#include "tests/common.h"
#include "tests/test_define.h"

static driver_test_cases_t test_case;
static ta_core_t ta_core;
struct timespec start_time, end_time;

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

void test_send_transfer(void) {
  const char* json_template =
      "{\"value\":100,"
      "\"message_format\":\"trytes\","
      "\"message\":\"%s\",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TRYTES_81_1 "\"}";
  tryte_t test_transfer_message[TEST_TRANSFER_MESSAGE_LEN + 1] = {};
  gen_rand_trytes(TEST_TRANSFER_MESSAGE_LEN, test_transfer_message);
  const int len = strlen(json_template) + TEST_TRANSFER_MESSAGE_LEN + 1;
  char* json = (char*)malloc(sizeof(char) * len);
  snprintf(json, len, json_template, test_transfer_message);
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_send_transfer(&ta_core, &iota_service, json, &json_result));
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
  free(json);
  printf("Average time of send_transfer: %lf\n", sum / TEST_COUNT);
}

void test_send_trytes(void) {
  const char* json = "{\"trytes\":[\"" TRYTES_2673_1 "\",\"" TRYTES_2673_2 "\"]}";
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            api_send_trytes(&ta_core.ta_conf, &ta_core.iota_conf, &iota_service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of send_trytes: %lf\n", sum / TEST_COUNT);
}

void test_find_transaction_objects(void) {
  const char* pre_json = "{\"hashes\":[\"%s\",\"%s\"]}";
  int json_len = (NUM_TRYTES_HASH + 1) * 2 + strlen(pre_json) + 1;
  char* json = (char*)malloc(sizeof(char) * json_len);
  snprintf(json, json_len, pre_json, test_case.txn_hash[0], test_case.txn_hash[1]);
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transaction_objects(&iota_service, json, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_transaction_objects: %lf\n", sum / TEST_COUNT);
  free(json);
}

void test_find_transactions_by_tag(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_by_tag(&iota_service, test_case.tag, &json_result));
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
    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_by_id(&iota_service, &ta_core.db_service,
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

    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_obj_by_tag(&iota_service, test_case.tag, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of find_tx_obj_by_tag: %lf\n", sum / TEST_COUNT);
}

void test_fetch_buffered_request_status(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_fetch_buffered_request_status(&ta_core.cache, TEST_UUID, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of fetch_buffered_request_status: %lf\n", sum / TEST_COUNT);
}

void test_get_node_status(void) {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    iota_client_service_t iota_service;
    ta_set_iota_client_service(&iota_service, ta_core.iota_service.http.host, ta_core.iota_service.http.port,
                               ta_core.iota_service.http.ca_pem);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_get_node_status(&iota_service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_node_status: %lf\n", sum / TEST_COUNT);
}

int main(int argc, char* argv[]) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  ta_core_default_init(&ta_core);
  driver_core_cli_init(&ta_core, argc, argv, &test_case);
  ta_core_set(&ta_core);
  ta_logger_switch(false, true, &ta_core.ta_conf);

  printf("Total samples for each API test: %d\n", TEST_COUNT);
  RUN_TEST(test_send_transfer);
  RUN_TEST(test_send_trytes);
  RUN_TEST(test_find_transaction_objects);
  RUN_TEST(test_find_transactions_by_tag);
  RUN_TEST(test_find_transactions_obj_by_tag);
#ifdef DB_ENABLE
  RUN_TEST(test_api_get_identity_info_by_hash);
  RUN_TEST(test_api_get_identity_info_by_id);
  RUN_TEST(test_find_transactions_by_id);
#endif
  // TODO recover proxy tests
  // RUN_TEST(test_proxy_apis);
  RUN_TEST(test_get_node_status);
  RUN_TEST(test_fetch_buffered_request_status);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
