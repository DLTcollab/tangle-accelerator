/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "serializer/serializer.h"
#include "test_define.h"

void test_serialize_ta_generate_address(void) {
  const char* json = "[\"" TRYTES_81_1 "\",\"" TRYTES_81_2 "\"]";
  char* json_result;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243], hash_trits_2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(&res->addresses, hash_trits_1);
  hash243_queue_push(&res->addresses, hash_trits_2);

  ta_generate_address_res_serialize(res, &json_result);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_generate_address_res_free(&res);
  free(json_result);
}

void test_deserialize_ta_send_transfer(void) {
  const char* json =
      "{\"value\":100,"
      "\"message_format\":\"trytes\","
      "\"message\":\"" TAG_MSG "\",\"tag\":\"" TAG_MSG
      "\","
      "\"address\":\"" TRYTES_81_1 "\"}";

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  flex_trit_t tag_msg_trits[FLEX_TRIT_SIZE_81], hash_trits_1[FLEX_TRIT_SIZE_243];

  ta_send_transfer_req_deserialize(json, req);

  TEST_ASSERT_EQUAL_INT(100, req->value);
  flex_trits_from_trytes(tag_msg_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  TEST_ASSERT_EQUAL_MEMORY(tag_msg_trits, req->tag->hash, FLEX_TRIT_SIZE_81);
  TEST_ASSERT_EQUAL_MEMORY(tag_msg_trits, req->message, FLEX_TRIT_SIZE_81);

  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(hash_trits_1, req->address->hash, FLEX_TRIT_SIZE_243);

  ta_send_transfer_req_free(&req);
}

void test_serialize_ta_find_transaction_objects(void) {
  const char* json =
      "[{\"hash\":\"" TRYTES_81_1 "\","
      "\"signature_and_message_fragment\":\"" TRYTES_2187_1 "\","
      "\"address\":\"" TRYTES_81_1 "\",\"value\":" STR(VALUE) ","
      "\"obsolete_tag\":\"" TAG_MSG "\",\"timestamp\":" STR(TIMESTAMP) ","
      "\"current_index\":" STR(CURRENT_INDEX) ",\"last_index\":" STR(LAST_INDEX) ","
      "\"bundle_hash\":\"" TRYTES_81_2 "\","
      "\"trunk_transaction_hash\":\"" TRYTES_81_2 "\","
      "\"branch_transaction_hash\":\"" TRYTES_81_1 "\","
      "\"tag\":\"" TAG_MSG "\","
      "\"attachment_timestamp\":" STR(TIMESTAMP) ","
      "\"attachment_timestamp_lower_bound\":" STR(TIMESTAMP)","
      "\"attachment_timestamp_upper_bound\":" STR(TIMESTAMP)","
      "\"nonce\":\"" NONCE "\"}]";
  char* json_result;
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561], tag_trits[FLEX_TRIT_SIZE_81], hash_trits_1[FLEX_TRIT_SIZE_243],
      hash_trits_2[FLEX_TRIT_SIZE_243];
  // ta_find_transaction_objects_res_t* res = ta_find_transaction_objects_res_new();
  transaction_array_t* res = transaction_array_new();

  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  iota_transaction_t* txn = transaction_new();
  // set transaction hash
  transaction_set_hash(txn, hash_trits_1);

  // set message
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE, (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  transaction_set_signature(txn, msg_trits);

  // set address
  transaction_set_address(txn, hash_trits_1);
  // set value
  transaction_set_value(txn, VALUE);

  // set obsolete_tag
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  transaction_set_obsolete_tag(txn, tag_trits);

  // set timestamp
  transaction_set_timestamp(txn, TIMESTAMP);
  // set current_index
  transaction_set_current_index(txn, CURRENT_INDEX);
  // set last_index
  transaction_set_last_index(txn, LAST_INDEX);
  // set bundle_hash
  transaction_set_bundle(txn, hash_trits_2);
  // set trunk
  transaction_set_trunk(txn, hash_trits_2);
  // set branch
  transaction_set_branch(txn, hash_trits_1);
  // set tag
  transaction_set_tag(txn, tag_trits);
  // set attachment_timestamp
  transaction_set_attachment_timestamp(txn, TIMESTAMP);
  // set attachment_timestamp_lower_bound
  transaction_set_attachment_timestamp_lower(txn, TIMESTAMP);
  // set attachment_timestamp_upper_bound
  transaction_set_attachment_timestamp_upper(txn, TIMESTAMP);
  // set nonce
  flex_trits_from_trytes(tag_trits, NUM_TRITS_NONCE, (const tryte_t*)NONCE, NUM_TRYTES_NONCE, NUM_TRYTES_NONCE);
  transaction_set_nonce(txn, tag_trits);
  transaction_array_push_back(res, txn);

  ta_find_transaction_objects_res_serialize(res, &json_result);

  TEST_ASSERT_EQUAL_STRING(json, json_result);
  transaction_array_free(res);
  transaction_free(txn);
  free(json_result);
}

void test_serialize_ta_find_transactions_by_tag(void) {
  const char* json = "[\"" TRYTES_81_1 "\",\"" TRYTES_81_2 "\"]";
  char* json_result;
  ta_find_transactions_by_tag_res_t* res = ta_find_transactions_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243], hash_trits_2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);

  hash243_queue_push(&res->hashes, hash_trits_1);
  hash243_queue_push(&res->hashes, hash_trits_2);

  ta_find_transactions_by_tag_res_serialize(res, &json_result);

  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_find_transactions_res_free(&res);
  free(json_result);
}

void test_serialize_ta_find_transactions_obj_by_tag(void) {
  const char* json =
      "[{\"hash\":\"" TRYTES_81_1 "\","
      "\"signature_and_message_fragment\":\"" TRYTES_2187_1 "\","
      "\"address\":\"" TRYTES_81_1 "\",\"value\":" STR(VALUE) ","
      "\"obsolete_tag\":\"" TAG_MSG "\",\"timestamp\":" STR(TIMESTAMP) ","
      "\"current_index\":" STR(CURRENT_INDEX) ",\"last_index\":" STR(LAST_INDEX) ","
      "\"bundle_hash\":\"" TRYTES_81_2 "\","
      "\"trunk_transaction_hash\":\"" TRYTES_81_2 "\","
      "\"branch_transaction_hash\":\"" TRYTES_81_1 "\","
      "\"tag\":\"" TAG_MSG "\","
      "\"attachment_timestamp\":" STR(TIMESTAMP) ","
      "\"attachment_timestamp_lower_bound\":" STR(TIMESTAMP)","
      "\"attachment_timestamp_upper_bound\":" STR(TIMESTAMP)","
      "\"nonce\":\"" NONCE "\"}]";
  char* json_result;
  iota_transaction_t* txn = transaction_new();
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561], tag_trits[FLEX_TRIT_SIZE_81], hash1[FLEX_TRIT_SIZE_243],
      hash2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  ta_find_transactions_obj_res_t* res = ta_find_transactions_obj_res_new();

  // set transaction hash
  transaction_set_hash(txn, hash1);

  // set message
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE, (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  transaction_set_signature(txn, msg_trits);

  // set address
  transaction_set_address(txn, hash1);
  // set value
  transaction_set_value(txn, VALUE);

  // set obsolete_tag
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  transaction_set_obsolete_tag(txn, tag_trits);

  // set timestamp
  transaction_set_timestamp(txn, TIMESTAMP);
  // set current_index
  transaction_set_current_index(txn, CURRENT_INDEX);
  // set last_index
  transaction_set_last_index(txn, LAST_INDEX);
  // set bundle_hash
  transaction_set_bundle(txn, hash2);
  // set trunk
  transaction_set_trunk(txn, hash2);
  // set branch
  transaction_set_branch(txn, hash1);
  // set tag
  transaction_set_tag(txn, tag_trits);
  // set attachment_timestamp
  transaction_set_attachment_timestamp(txn, TIMESTAMP);
  // set attachment_timestamp_lower_bound
  transaction_set_attachment_timestamp_lower(txn, TIMESTAMP);
  // set attachment_timestamp_upper_bound
  transaction_set_attachment_timestamp_upper(txn, TIMESTAMP);
  // set nonce
  flex_trits_from_trytes(tag_trits, NUM_TRITS_NONCE, (const tryte_t*)NONCE, NUM_TRYTES_NONCE, NUM_TRYTES_NONCE);
  transaction_set_nonce(txn, tag_trits);

  utarray_push_back(res->txn_obj, txn);
  ta_find_transaction_objects_res_serialize(res->txn_obj, &json_result);

  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_find_transactions_obj_res_free(&res);
  transaction_free(txn);
  free(json_result);
}

void test_serialize_send_mam_message(void) {
  const char* json = "{\"channel\":\"" TRYTES_81_1
                     "\","
                     "\"bundle_hash\":\"" TRYTES_81_2 "\",\"channel_ord\":" STR(TEST_CHANNEL_ORD) "}";
  char* json_result;
  ta_send_mam_res_t* res = send_mam_res_new();

  send_mam_res_set_bundle_hash(res, (tryte_t*)TRYTES_81_2);
  send_mam_res_set_channel_id(res, (tryte_t*)TRYTES_81_1);
  res->channel_ord = TEST_CHANNEL_ORD;

  send_mam_res_serialize(res, &json_result);
  TEST_ASSERT_EQUAL_STRING(json, json_result);

  free(json_result);
  send_mam_res_free(&res);
}

void test_deserialize_send_mam_message_response(void) {
  const char* json = "{\"channel\":\"" TRYTES_81_1
                     "\","
                     "\"bundle_hash\":\"" TRYTES_81_2 "\"}";
  ta_send_mam_res_t* res = send_mam_res_new();

  send_mam_res_deserialize(json, res);

  TEST_ASSERT_EQUAL_STRING(TRYTES_81_1, res->channel_id);
  TEST_ASSERT_EQUAL_STRING(TRYTES_81_2, res->bundle_hash);

  send_mam_res_free(&res);
}

void test_deserialize_send_mam_message(void) {
  const char* json = "{\"prng\":\"" TRYTES_81_1 "\",\"message\":\"" TEST_PAYLOAD "\",\"channel_ord\":2}";
  ta_send_mam_req_t* req = send_mam_req_new();

  send_mam_req_deserialize(json, req);
  TEST_ASSERT_EQUAL_STRING(TRYTES_81_1, req->prng);
  TEST_ASSERT_EQUAL_STRING(TEST_PAYLOAD, req->payload);
  TEST_ASSERT_EQUAL_INT(2, req->channel_ord);

  send_mam_req_free(&req);
}

void test_deserialize_ta_send_trytes_req(void) {
  const char* json = "{\"trytes\":[\"" TRYTES_2673_1 "\",\"" TRYTES_2673_2 "\"]}";
  hash8019_array_p out_trytes = hash8019_array_new();
  ta_send_trytes_req_deserialize(json, out_trytes);

  flex_trit_t hash[FLEX_TRIT_SIZE_8019] = {};
  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  TEST_ASSERT_EQUAL_MEMORY(hash, hash_array_at(out_trytes, 0), NUM_TRYTES_SERIALIZED_TRANSACTION);

  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_2,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  TEST_ASSERT_EQUAL_MEMORY(hash, hash_array_at(out_trytes, 1), NUM_TRYTES_SERIALIZED_TRANSACTION);

  hash_array_free(out_trytes);
}

void test_serialize_ta_send_trytes_res(void) {
  const char* json = "{\"trytes\":[\"" TRYTES_2673_1 "\",\"" TRYTES_2673_2 "\"]}";
  char* json_result;
  hash8019_array_p trytes = hash8019_array_new();

  flex_trit_t hash[FLEX_TRIT_SIZE_8019] = {};
  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  hash_array_push(trytes, hash);

  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_2,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  hash_array_push(trytes, hash);

  ta_send_trytes_res_serialize(trytes, &json_result);

  TEST_ASSERT_EQUAL_STRING(json, json_result);

  hash_array_free(trytes);
  free(json_result);
}

void test_mqtt_device_id_deserialize(void) {
  const char* json = "{\"device_id\":\"" DEVICE_ID "\", \"trytes\":[\"" TRYTES_2673_1 "\",\"" TRYTES_2673_2 "\"]}";
  const int id_len = 32;
  char device_id[id_len + 1];
  TEST_ASSERT_EQUAL_INT(SC_OK, mqtt_device_id_deserialize(json, device_id));

  TEST_ASSERT_EQUAL_STRING(device_id, DEVICE_ID);
}

void test_mqtt_tag_req_deserialize(void) {
  const char* json = "{\"device_id\":\"" DEVICE_ID "\", \"tag\":\"" TAG_MSG "\"}";
  char tag[NUM_TRYTES_TAG + 1];
  TEST_ASSERT_EQUAL_INT(SC_OK, mqtt_tag_req_deserialize(json, tag));

  TEST_ASSERT_EQUAL_STRING(tag, TAG_MSG);
}

void test_mqtt_transaction_hash_req_deserialize(void) {
  const char* json = "{\"device_id\":\"" DEVICE_ID "\", \"hash\":\"" TRYTES_81_1 "\"}";
  char hash[NUM_TRYTES_HASH + 1];
  TEST_ASSERT_EQUAL_INT(SC_OK, mqtt_transaction_hash_req_deserialize(json, hash));

  TEST_ASSERT_EQUAL_STRING(hash, TRYTES_81_1);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_serialize_ta_generate_address);
  RUN_TEST(test_deserialize_ta_send_transfer);
  RUN_TEST(test_serialize_ta_find_transaction_objects);
  RUN_TEST(test_serialize_ta_find_transactions_by_tag);
  RUN_TEST(test_serialize_ta_find_transactions_obj_by_tag);
  RUN_TEST(test_serialize_send_mam_message);
  RUN_TEST(test_deserialize_send_mam_message_response);
  RUN_TEST(test_deserialize_send_mam_message);
  RUN_TEST(test_deserialize_ta_send_trytes_req);
  RUN_TEST(test_serialize_ta_send_trytes_res);
  RUN_TEST(test_mqtt_device_id_deserialize);
  RUN_TEST(test_mqtt_tag_req_deserialize);
  RUN_TEST(test_mqtt_transaction_hash_req_deserialize);
  return UNITY_END();
}
