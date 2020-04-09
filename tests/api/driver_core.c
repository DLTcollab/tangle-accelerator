/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "accelerator/core/core.h"
#include "common.h"
#include "tests/test_define.h"

static ta_core_t ta_core;

status_t prepare_transfer(const iota_config_t* const iconf, const iota_client_service_t* const service,
                          ta_send_transfer_req_t* req, hash8019_array_p raw_txn_array) {
  status_t ret = SC_OK;
  bundle_transactions_t* out_bundle = NULL;
  bundle_transactions_new(&out_bundle);
  transfer_array_t* transfers = transfer_array_new();
  if (transfers == NULL) {
    ret = SC_CCLIENT_OOM;
    printf("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }

  tryte_t msg_tryte[NUM_TRYTES_SERIALIZED_TRANSACTION];
  flex_trits_to_trytes(msg_tryte, req->msg_len / 3, req->message, req->msg_len, req->msg_len);

  transfer_t transfer = {.value = 0, .timestamp = current_timestamp_ms(), .msg_len = req->msg_len / 3};

  if (transfer_message_set_trytes(&transfer, msg_tryte, transfer.msg_len) != RC_OK) {
    ret = SC_CCLIENT_OOM;
    printf("%s\n", "SC_CCLIENT_OOM");
    goto done;
  }
  memcpy(transfer.address, hash243_queue_peek(req->address), FLEX_TRIT_SIZE_243);
  memcpy(transfer.tag, hash81_queue_peek(req->tag), FLEX_TRIT_SIZE_81);
  transfer_array_add(transfers, &transfer);

  // TODO we may need args `remainder_address`, `inputs`, `timestampe` in the
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
  bundle_transactions_free(&out_bundle);
  transfer_message_free(&transfer);
  transfer_array_free(transfers);
  return ret;
}

void test_broadcast_buffered_txn(void) {
  // Generate transaction trytes, and don't send them
  const char* json =
      "{\"value\":100"
      ",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TEST_ADDRESS
      "\","
      "\"message\":\"" TEST_TRANSFER_MESSAGE "\"}";

  char uuid[UUID_STR_LEN];
  int list_len = -1;
  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  hash8019_array_p raw_txn_array = hash8019_array_new();

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.buffer_list_name, &list_len));
  const int init_list_len = list_len;

  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_send_transfer_req_deserialize(json, req));
  TEST_ASSERT_EQUAL_INT32(SC_OK, prepare_transfer(&ta_core.iota_conf, &ta_core.iota_service, req, raw_txn_array));

  // Push transaction trytes to redis server
  TEST_ASSERT_EQUAL_INT32(SC_OK, push_txn_to_buffer(&ta_core.cache, raw_txn_array, uuid));

  flex_trit_t txn_flex_trits[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1] = {};
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_get(uuid, (char*)txn_flex_trits));
  TEST_ASSERT_FALSE(flex_trits_are_null(txn_flex_trits, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION));

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.buffer_list_name, &list_len));
  TEST_ASSERT_EQUAL_INT32(init_list_len + 1, list_len);

  // Take action to broadcast transctions in buffer
  TEST_ASSERT_EQUAL_INT32(SC_OK, broadcast_buffered_txn(&ta_core));

  // The length of the buffer list should be zero, since all the transaction objects have been popped out.
  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_list_size(ta_core.cache.buffer_list_name, &list_len));
  TEST_ASSERT_EQUAL_INT32(init_list_len, list_len);

  TEST_ASSERT_EQUAL_INT32(SC_OK, cache_get(uuid, (char*)txn_flex_trits));
  TEST_ASSERT_FALSE(flex_trits_are_null(txn_flex_trits, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION));

  ta_send_transfer_req_free(&req);
  hash_array_free(raw_txn_array);
}

void test_fetch_txn_with_uuid(void) {
  // Generate transaction trytes, and don't send them
  const char* json =
      "{\"value\":100"
      ",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TEST_ADDRESS
      "\","
      "\"message\":\"" TEST_TRANSFER_MESSAGE "\"}";

  char uuid[UUID_STR_LEN];
  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  hash8019_array_p raw_txn_array = hash8019_array_new();
  ta_fetch_txn_with_uuid_res_t* res = ta_fetch_txn_with_uuid_res_new();
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_fetch_txn_with_uuid(&ta_core.cache, TEST_UUID, res));
  TEST_ASSERT_EQUAL_INT32(NOT_EXIST, res->status);

  // Preparing transaction
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_send_transfer_req_deserialize(json, req));
  TEST_ASSERT_EQUAL_INT32(SC_OK, prepare_transfer(&ta_core.iota_conf, &ta_core.iota_service, req, raw_txn_array));

  // Push transaction trytes to redis server
  TEST_ASSERT_EQUAL_INT32(SC_OK, push_txn_to_buffer(&ta_core.cache, raw_txn_array, uuid));
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_fetch_txn_with_uuid(&ta_core.cache, uuid, res));
  TEST_ASSERT_EQUAL_INT32(UNSENT, res->status);

  // Take action to broadcast transctions in buffer
  TEST_ASSERT_EQUAL_INT32(SC_OK, broadcast_buffered_txn(&ta_core));
  TEST_ASSERT_EQUAL_INT32(SC_OK, ta_fetch_txn_with_uuid(&ta_core.cache, uuid, res));
  TEST_ASSERT_EQUAL_INT32(SENT, res->status);
  tryte_t all[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {};
  flex_trit_t* transaction_flex_trits = transaction_serialize(res->txn);
  flex_trits_to_trytes(all, NUM_TRYTES_SERIALIZED_TRANSACTION, transaction_flex_trits, NUM_TRITS_SERIALIZED_TRANSACTION,
                       NUM_TRITS_SERIALIZED_TRANSACTION);
  tryte_t trytes[NUM_TRYTES_MESSAGE + 1] = {};
  flex_trits_to_trytes(trytes, NUM_TRYTES_TAG, transaction_tag(res->txn), NUM_TRITS_TAG, NUM_TRITS_TAG);
  TEST_ASSERT_EQUAL_STRING(TEST_TAG, (char*)trytes);
  flex_trits_to_trytes(trytes, NUM_TRYTES_HASH, transaction_address(res->txn), NUM_TRITS_HASH, NUM_TRITS_HASH);
  TEST_ASSERT_EQUAL_STRING(TEST_ADDRESS, (char*)trytes);
  flex_trits_to_trytes(trytes, NUM_TRYTES_MESSAGE, transaction_message(res->txn), NUM_TRITS_MESSAGE, NUM_TRITS_MESSAGE);
  char txn_msg_ascii[NUM_TRYTES_MESSAGE / 2 + 1] = {};
  trytes_to_ascii(trytes, NUM_TRYTES_MESSAGE, txn_msg_ascii);
  TEST_ASSERT_EQUAL_STRING(TEST_TRANSFER_MESSAGE, txn_msg_ascii);

  ta_send_transfer_req_free(&req);
  hash_array_free(raw_txn_array);
  ta_fetch_txn_with_uuid_res_free(&res);
  free(transaction_flex_trits);
}

int main(int argc, char* argv[]) {
  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }
  cc_logger_init();
  pow_logger_init();
  timer_logger_init();

  ta_core_default_init(&ta_core);
  driver_core_cli_init(&ta_core, argc, argv, NULL);
  ta_core_set(&ta_core);

  UNITY_BEGIN();
  RUN_TEST(test_broadcast_buffered_txn);
  RUN_TEST(test_fetch_txn_with_uuid);
  ta_core_destroy(&ta_core);
  return UNITY_END();
}
