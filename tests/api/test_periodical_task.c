/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "accelerator/core/apis.h"
#include "accelerator/core/core.h"
#include "accelerator/core/periodical_task.h"
#include "tests/common.h"
#include "tests/test_define.h"

#define TXN_NUM_IN_BUNDLE 2
#define MAM_REQ_NUM 2
#define ORDERED_PAYLOAD "test payload number"

static ta_core_t ta_core;
static char* response_uuid[MAM_REQ_NUM];

status_t prepare_transfer(const iota_config_t* const iconf, const iota_client_service_t* const service,
                          ta_send_transfer_req_t** req, const int req_txn_num, hash8019_array_p raw_txn_array) {
  status_t ret = SC_OK;
  bundle_transactions_t* out_bundle = NULL;
  bundle_transactions_new(&out_bundle);
  transfer_array_t* transfers = transfer_array_new();
  transfer_t transfer[req_txn_num];
  if (transfers == NULL) {
    ret = SC_OOM;
    printf("%s\n", "SC_OOM");
    goto done;
  }

  for (int i = 0; i < req_txn_num; ++i) {
    transfer[i].value = 0;
    transfer[i].timestamp = current_timestamp_ms();
    transfer[i].msg_len = req[i]->msg_len;

    if (transfer_message_set_trytes(&transfer[i], req[i]->message, transfer[i].msg_len) != RC_OK) {
      ret = SC_OOM;
      printf("%s\n", ta_error_to_string(SC_OOM));
      goto done;
    }
    memcpy(transfer[i].address, hash243_queue_peek(req[i]->address), FLEX_TRIT_SIZE_243);
    memcpy(transfer[i].tag, hash81_queue_peek(req[i]->tag), FLEX_TRIT_SIZE_81);
    transfer_array_add(transfers, &transfer[i]);
  }

  // TODO we may need args `remainder_address`, `inputs`, `timestamp` in the
  // future and declare `security` field in `iota_config_t`
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(seed, NUM_TRITS_HASH, (tryte_t const*)iconf->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  if (iota_client_prepare_transfers(service, seed, 2, transfers, NULL, NULL, false, current_timestamp_ms(),
                                    out_bundle) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    printf("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  iota_transaction_t* txn = NULL;
  BUNDLE_FOREACH(out_bundle, txn) {
    flex_trit_t* serialized_txn = transaction_serialize(txn);
    if (serialized_txn == NULL) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      printf("%s\n", "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
    hash_array_push(raw_txn_array, serialized_txn);
    free(serialized_txn);
  }

done:
  for (int i = 0; i < req_txn_num; i++) {
    transfer_message_free(&transfer[i]);
  }

  bundle_transactions_free(&out_bundle);
  transfer_array_free(transfers);
  return ret;
}

void test_broadcast_buffered_txn(void) {
  // Generate transaction trytes, and don't send them
  const char* json_template =
      "{\"value\":100,"
      "\"message_format\":\"trytes\","
      "\"message\":\"%s\",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TEST_ADDRESS "\"}";
  tryte_t test_transfer_message[TXN_NUM_IN_BUNDLE][TEST_TRANSFER_MESSAGE_LEN + 1] = {};
  const int len = strlen(json_template) + TEST_TRANSFER_MESSAGE_LEN;
  char* json[TXN_NUM_IN_BUNDLE];
  ta_send_transfer_req_t* req[TXN_NUM_IN_BUNDLE];
  for (int i = 0; i < TXN_NUM_IN_BUNDLE; i++) {
    gen_rand_trytes(TEST_TRANSFER_MESSAGE_LEN, test_transfer_message[i]);
    json[i] = (char*)malloc(sizeof(char) * len);
    snprintf(json[i], len, json_template, test_transfer_message[i]);
    req[i] = ta_send_transfer_req_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, ta_send_transfer_req_deserialize(json[i], req[i]));
  }

  char uuid[UUID_STR_LEN];
  int list_len = -1;

  hash8019_array_p raw_txn_array = hash8019_array_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.buffer_list_name, &list_len));
  const int init_list_len = list_len;

  TEST_ASSERT_EQUAL_INT32(
      SC_OK, prepare_transfer(&ta_core.iota_conf, &ta_core.iota_service, req, TXN_NUM_IN_BUNDLE, raw_txn_array));

  // Push transaction trytes to redis server
  TEST_ASSERT_EQUAL_INT32(SC_OK, push_txn_to_buffer(&ta_core.cache, raw_txn_array, uuid));

  int bundle_len = 0;
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(uuid, &bundle_len));
  TEST_ASSERT_TRUE(bundle_len);
  for (int i = 0; i < bundle_len; ++i) {
    flex_trit_t txn_flex_trits[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1] = {};
    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            cache_list_at(uuid, i, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION, (char*)txn_flex_trits));
    TEST_ASSERT_FALSE(flex_trits_are_null(txn_flex_trits, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION));
  }

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.buffer_list_name, &list_len));
  TEST_ASSERT_EQUAL_INT32(init_list_len + 1, list_len);

  // Take action to broadcast transactions in buffer
  TEST_ASSERT_EQUAL_INT32(SC_OK, broadcast_buffered_txn(&ta_core));

  // The length of the buffer list should be zero, since all the transaction objects have been popped out.
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.buffer_list_name, &list_len));
  TEST_ASSERT_EQUAL_INT32(init_list_len, list_len);

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(uuid, &bundle_len));
  TEST_ASSERT_EQUAL_INT32(TXN_NUM_IN_BUNDLE, bundle_len);
  for (int i = 0; i < bundle_len; ++i) {
    flex_trit_t txn_flex_trits[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1] = {};
    TEST_ASSERT_EQUAL_INT32(SC_OK,
                            cache_list_at(uuid, i, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION, (char*)txn_flex_trits));
    TEST_ASSERT_FALSE(flex_trits_are_null(txn_flex_trits, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION));
  }

  for (int i = 0; i < TXN_NUM_IN_BUNDLE; i++) {
    ta_send_transfer_req_free(&req[i]);
    free(json[i]);
  }
  hash_array_free(raw_txn_array);
}

void test_fetch_buffered_request_status(void) {
  // Generate transaction trytes, and don't send them
  const char* json_template =
      "{\"value\":100,"
      "\"message_format\":\"trytes\","
      "\"message\":\"%s\",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TEST_ADDRESS "\"}";
  tryte_t test_transfer_message[TXN_NUM_IN_BUNDLE][TEST_TRANSFER_MESSAGE_LEN + 1] = {};
  const int len = strlen(json_template) + TEST_TRANSFER_MESSAGE_LEN;
  char* json[TXN_NUM_IN_BUNDLE];
  ta_send_transfer_req_t* req[TXN_NUM_IN_BUNDLE];
  hash8019_array_p raw_txn_array = hash8019_array_new();
  for (int i = 0; i < TXN_NUM_IN_BUNDLE; i++) {
    gen_rand_trytes(TEST_TRANSFER_MESSAGE_LEN, test_transfer_message[i]);
    json[i] = (char*)malloc(sizeof(char) * len);
    snprintf(json[i], len, json_template, test_transfer_message[i]);
    req[i] = ta_send_transfer_req_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, ta_send_transfer_req_deserialize(json[i], req[i]));
  }
  char uuid[UUID_STR_LEN];
  ta_fetch_buffered_request_status_res_t* res = ta_fetch_buffered_request_status_res_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_fetch_txn_with_uuid(&ta_core.cache, uuid, res));
  TEST_ASSERT_EQUAL_INT32(NOT_EXIST, res->status);

  TEST_ASSERT_EQUAL_INT32(
      SC_OK, prepare_transfer(&ta_core.iota_conf, &ta_core.iota_service, req, TXN_NUM_IN_BUNDLE, raw_txn_array));

  // Push transaction trytes to redis server
  TEST_ASSERT_EQUAL_INT32(SC_OK, push_txn_to_buffer(&ta_core.cache, raw_txn_array, uuid));
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_fetch_txn_with_uuid(&ta_core.cache, uuid, res));
  TEST_ASSERT_EQUAL_INT32(UNSENT, res->status);

  // Take action to broadcast transactions in buffer
  TEST_ASSERT_EQUAL_INT32(SC_OK, broadcast_buffered_txn(&ta_core));
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_fetch_txn_with_uuid(&ta_core.cache, uuid, res));
  TEST_ASSERT_EQUAL_INT32(SENT, res->status);

  TEST_ASSERT_EQUAL_INT32(2, bundle_transactions_size(res->bundle));

  for (int i = 0; i < TXN_NUM_IN_BUNDLE; ++i) {
    tryte_t all[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {};
    iota_transaction_t* txn = bundle_at(res->bundle, i);
    flex_trit_t* transaction_flex_trits = transaction_serialize(txn);
    flex_trits_to_trytes(all, NUM_TRYTES_SERIALIZED_TRANSACTION, transaction_flex_trits,
                         NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
    tryte_t trytes[NUM_TRYTES_MESSAGE + 1] = {};
    flex_trits_to_trytes(trytes, NUM_TRYTES_TAG, transaction_tag(txn), NUM_TRITS_TAG, NUM_TRITS_TAG);
    TEST_ASSERT_EQUAL_STRING(TEST_TAG, (char*)trytes);
    flex_trits_to_trytes(trytes, NUM_TRYTES_HASH, transaction_address(txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
    TEST_ASSERT_EQUAL_STRING(TEST_ADDRESS, (char*)trytes);
    flex_trits_to_trytes(trytes, NUM_TRYTES_MESSAGE, transaction_message(txn), NUM_TRITS_MESSAGE, NUM_TRITS_MESSAGE);
    TEST_ASSERT_EQUAL_MEMORY(test_transfer_message[i], trytes, TEST_TRANSFER_MESSAGE_LEN);
    free(transaction_flex_trits);
  }

  for (int i = 0; i < TXN_NUM_IN_BUNDLE; i++) {
    ta_send_transfer_req_free(&req[i]);
    free(json[i]);
  }

  hash_array_free(raw_txn_array);
  ta_fetch_buffered_request_status_res_free(&res);
}

void test_broadcast_buffered_mam(void) {
  const char* json_template = "{\"x-api-key\":\"" TEST_TOKEN "\",\"data\":{\"seed\":\"%s\",\"ch_mss_depth\":" STR(
      TEST_CH_DEPTH) ",\"message\":\"" ORDERED_PAYLOAD ":%d\"}, \"protocol\":\"MAM_V1\"}";
  char seed[NUM_TRYTES_ADDRESS + 1] = {};
  gen_rand_trytes(NUM_TRYTES_ADDRESS, (tryte_t*)seed);
  const int len = strlen(json_template) + NUM_TRYTES_ADDRESS;
  int list_len = -1;

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.mam_buffer_list_name, &list_len));
  const int init_list_len = list_len;

  for (int i = 0; i < MAM_REQ_NUM; i++) {
    char* json_result = NULL;
    char* json = (char*)malloc(sizeof(char) * len);
    snprintf(json, len, json_template, seed, i);
    ta_send_mam_req_t* req = send_mam_req_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_send_mam_message(&ta_core.cache, json, &json_result));
    response_uuid[i] = json_result;
    free(json);
    send_mam_req_free(&req);
  }

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.mam_buffer_list_name, &list_len));
  TEST_ASSERT_EQUAL_INT32(init_list_len + MAM_REQ_NUM, list_len);

  // Take action to broadcast transactions in buffer
  TEST_ASSERT_EQUAL_INT32(SC_OK, broadcast_buffered_send_mam_request(&ta_core));

  // The length of the buffer list should be zero, since all the transaction objects have been popped out.
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.mam_buffer_list_name, &list_len));
  TEST_ASSERT_EQUAL_INT32(init_list_len, list_len);
}

void test_mam_fetch_with_uuid(void) {
  for (int i = 0; i < MAM_REQ_NUM; i++) {
    char* json_result = NULL;
    char uuid[UUID_STR_LEN] = {};
    ta_fetch_buffered_request_status_res_t* res = ta_fetch_buffered_request_status_res_new();
    TEST_ASSERT_EQUAL_INT32(SC_OK, fetch_buffered_request_status_req_deserialize(response_uuid[i], uuid));
    TEST_ASSERT_EQUAL_INT32(SC_OK, api_fetch_buffered_request_status(&ta_core.cache, uuid, &json_result));
    free(json_result);
    ta_fetch_buffered_request_status_res_free(&res);
  }
}

int main(int argc, char* argv[]) {
  rand_trytes_init();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  ta_core_default_init(&ta_core);
  driver_core_cli_init(&ta_core, argc, argv, NULL);
  ta_core_set(&ta_core);
  ta_logger_switch(false, true, &ta_core.ta_conf);

  const int list_name_len = 15;
  ta_core.cache.mam_buffer_list_name = (char*)malloc(list_name_len);
  gen_rand_trytes(list_name_len - 1, (tryte_t*)ta_core.cache.mam_buffer_list_name);
  ta_core.cache.mam_buffer_list_name[list_name_len - 1] = 0;
  ta_core.cache.mam_complete_list_name = (char*)malloc(list_name_len);
  gen_rand_trytes(list_name_len - 1, (tryte_t*)ta_core.cache.mam_complete_list_name);
  ta_core.cache.mam_complete_list_name[list_name_len - 1] = 0;

  UNITY_BEGIN();
  RUN_TEST(test_broadcast_buffered_txn);
  RUN_TEST(test_fetch_buffered_request_status);
  RUN_TEST(test_broadcast_buffered_mam);
  RUN_TEST(test_mam_fetch_with_uuid);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
