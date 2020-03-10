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
#include "tests/test_define.h"

static ta_core_t ta_core;
struct timespec start_time, end_time;

char* test_txn_hash[2] = {NULL, NULL};
int test_txn_hash_size = sizeof(test_txn_hash) / sizeof(char*);
char* test_tag = NULL;
static struct ta_cli_argument_s driver_cli_arguments_g[] = {
    {"hash", required_argument, NULL, 's', "testing transaction hashes"},
    {"tag", required_argument, NULL, 'g', "testing transaction tag"},
    {NULL, 0, NULL, 0, NULL}};

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
  const char* json =
      "{\"value\":100"
      ",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TRYTES_81_1
      "\","
      "\"message\":\"" TEST_TRANSFER_MESSAGE "\"}";
  char* json_result;
  double sum = 0;

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
  const char* pre_json = "{\"hashes\":[\"%s\",\"%s\"]}";
  int json_len = (NUM_TRYTES_HASH + 1) * 2 + strlen(pre_json) + 1;
  char* json = (char*)malloc(sizeof(char) * json_len);
  snprintf(json, json_len, pre_json, test_txn_hash[0], test_txn_hash[1]);
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transaction_objects(&ta_core.iota_service, json, &json_result));
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

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_by_tag(&ta_core.iota_service, test_tag, &json_result));
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

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_find_transactions_obj_by_tag(&ta_core.iota_service, test_tag, &json_result));
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
    ret = api_send_mam_message(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, json, &json_result);
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
        SC_OK, api_recv_mam_message(&ta_core.iota_conf, &ta_core.iota_service, (char*)res->chid, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of receive_mam_message: %lf\n", sum / TEST_COUNT);
}

void test_get_iri_status() {
  char* json_result;
  double sum = 0;

  for (size_t count = 0; count < TEST_COUNT; count++) {
    test_time_start(&start_time);

    TEST_ASSERT_EQUAL_INT32(SC_OK, api_get_iri_status(&ta_core.iota_service, &json_result));
    test_time_end(&start_time, &end_time, &sum);
    free(json_result);
  }
  printf("Average time of get_iri_status: %lf\n", sum / TEST_COUNT);
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
static inline struct option* driver_cli_build_options() {
  int driver_cli_arg_size = sizeof(driver_cli_arguments_g) / sizeof(driver_cli_arguments_g[0]);
  struct option* driver_long_options =
      (struct option*)realloc(cli_build_options(), (cli_cmd_num + 2) * sizeof(struct option));
  for (int i = 0; i < driver_cli_arg_size; i++) {
    driver_long_options[(cli_cmd_num - 1) + i].name = driver_cli_arguments_g[i].name;
    driver_long_options[(cli_cmd_num - 1) + i].has_arg = driver_cli_arguments_g[i].has_arg;
    driver_long_options[(cli_cmd_num - 1) + i].flag = driver_cli_arguments_g[i].flag;
    driver_long_options[(cli_cmd_num - 1) + i].val = driver_cli_arguments_g[i].val;
  }

  return driver_long_options;
};
status_t driver_core_cli_init(ta_core_t* const core, int argc, char** argv) {
  int key = 0;
  status_t ret = SC_OK;
  struct option* long_options = driver_cli_build_options();

  while ((key = getopt_long(argc, argv, "hvs:g:", long_options, NULL)) != -1) {
    switch (key) {
      case 'h':
        ta_usage();
        exit(EXIT_SUCCESS);
      case 'v':
        printf("%s\n", TA_VERSION);
        exit(EXIT_SUCCESS);
      case QUIET:
        // Turn on quiet mode
        quiet_mode = true;

        // Enable backend_redis logger
        br_logger_init();
        break;
      case 's':
        // Take the arguments as testing transaction hashes
        for (int i = 0; i < test_txn_hash_size; i++) {
          if (!test_txn_hash[i]) {
            test_txn_hash[i] = optarg;
          }
        }
        break;
      case 'g':
        // Take the arguments as testing transaction tag
        test_tag = optarg;
        break;
      default:
        ret = cli_core_set(core, key, optarg);
        break;
    }
    if (ret != SC_OK) {
      break;
    }
  }

  free(long_options);
  return ret;
}

int main(int argc, char* argv[]) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }
  apis_logger_init();
  cc_logger_init();
  pow_logger_init();
  timer_logger_init();

  ta_core_default_init(&ta_core);
  driver_core_cli_init(&ta_core, argc, argv);
  ta_core_set(&ta_core);

  printf("Total samples for each API test: %d\n", TEST_COUNT);

  RUN_TEST(test_generate_address);
  RUN_TEST(test_get_tips_pair);
  RUN_TEST(test_get_tips);
  RUN_TEST(test_send_transfer);
  RUN_TEST(test_send_trytes);
  RUN_TEST(test_find_transaction_objects);
  // RUN_TEST(test_send_mam_message);
  // RUN_TEST(test_receive_mam_message);
  RUN_TEST(test_find_transactions_by_tag);
  RUN_TEST(test_find_transactions_obj_by_tag);
#ifdef DB_ENABLE
  RUN_TEST(test_api_get_identity_info_by_hash);
  RUN_TEST(test_api_get_identity_info_by_id);
  RUN_TEST(test_find_transactions_by_id);
#endif
  // TODO recover proxy tests
  // RUN_TEST(test_proxy_apis);
  RUN_TEST(test_get_iri_status);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
