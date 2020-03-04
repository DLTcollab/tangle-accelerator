/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifdef MQTT_ENABLE
#include "connectivity/mqtt/mqtt_common.h"
#endif
#include "accelerator/core/serializer/serializer.h"
#include "tests/test_define.h"

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

void test_deserialize_ta_send_transfer_zero(void) {
  const char* json =
      "{\"value\":100,"
      "\"message_format\":\"trytes\","
      "\"message\":\"" TEST_TRANSFER_MESSAGE "\",\"tag\":\"" TEST_TAG
      "\","
      "\"address\":\"" TRYTES_81_1 "\"}";

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  tryte_t tag_trytes[NUM_TRYTES_TAG + 1] = {0}, hash_trytes[NUM_TRYTES_HASH + 1] = {0};
  char msg_trytes[NUM_TRYTES_TAG + 1] = {0};

  ta_send_transfer_req_deserialize(json, req);

  TEST_ASSERT_EQUAL_INT(0, req->value);
  flex_trits_to_trytes(tag_trytes, NUM_TRYTES_TAG, req->tag->hash, NUM_TRITS_TAG, NUM_TRITS_TAG);
  TEST_ASSERT_EQUAL_STRING(TEST_TAG, tag_trytes);

  trytes_to_ascii(req->message, NUM_TRYTES_TAG * 2, msg_trytes);
  TEST_ASSERT_EQUAL_STRING(TEST_TRANSFER_MESSAGE, (char*)msg_trytes);

  flex_trits_to_trytes(hash_trytes, NUM_TRYTES_HASH, (const tryte_t*)req->address->hash, NUM_TRITS_HASH,
                       NUM_TRITS_HASH);
  TEST_ASSERT_EQUAL_STRING(TRYTES_81_1, hash_trytes);

  ta_send_transfer_req_free(&req);
}

void test_deserialize_ta_send_transfer_value(void) {
  const char* json = "{\"value\":100,\"seed\":\"" TRYTES_81_1
                     "\","
                     "\"message_format\":\"trytes\","
                     "\"message\":\"" TEST_TRANSFER_MESSAGE "\",\"tag\":\"" TEST_TAG
                     "\","
                     "\"address\":\"" TRYTES_81_2 "\"}";

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  tryte_t tag_trytes[NUM_TRYTES_TAG + 1] = {0}, hash_trytes[NUM_TRYTES_HASH + 1] = {0};
  char msg_trytes[NUM_TRYTES_TAG + 1] = {0};

  ta_send_transfer_req_deserialize(json, req);

  TEST_ASSERT_EQUAL_INT(100, req->value);
  flex_trits_to_trytes(tag_trytes, NUM_TRYTES_TAG, req->tag->hash, NUM_TRITS_TAG, NUM_TRITS_TAG);
  TEST_ASSERT_EQUAL_STRING(TEST_TAG, tag_trytes);

  trytes_to_ascii(req->message, NUM_TRYTES_TAG * 2, msg_trytes);
  TEST_ASSERT_EQUAL_STRING(TEST_TRANSFER_MESSAGE, (char*)msg_trytes);

  flex_trits_to_trytes(hash_trytes, NUM_TRYTES_HASH, (const tryte_t*)req->seed, NUM_TRITS_HASH, NUM_TRITS_HASH);
  TEST_ASSERT_EQUAL_STRING(TRYTES_81_1, hash_trytes);
  flex_trits_to_trytes(hash_trytes, NUM_TRYTES_HASH, (const tryte_t*)req->address->hash, NUM_TRITS_HASH,
                       NUM_TRITS_HASH);
  TEST_ASSERT_EQUAL_STRING(TRYTES_81_2, hash_trytes);

  ta_send_transfer_req_free(&req);
}

void test_serialize_ta_find_transaction_objects(void) {
  const char* json =
      "[{\"hash\":\"" TRYTES_81_1 "\","
      "\"signature_and_message_fragment\":\"" TRYTES_2187_1 "\","
      "\"address\":\"" TRYTES_81_1 "\",\"value\":" STR(VALUE) ","
      "\"obsolete_tag\":\"" TEST_TAG "\",\"timestamp\":" STR(TIMESTAMP) ","
      "\"current_index\":" STR(CURRENT_INDEX) ",\"last_index\":" STR(LAST_INDEX) ","
      "\"bundle_hash\":\"" TRYTES_81_2 "\","
      "\"trunk_transaction_hash\":\"" TRYTES_81_2 "\","
      "\"branch_transaction_hash\":\"" TRYTES_81_1 "\","
      "\"tag\":\"" TEST_TAG "\","
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
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TEST_TAG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
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
      "\"obsolete_tag\":\"" TEST_TAG "\",\"timestamp\":" STR(TIMESTAMP) ","
      "\"current_index\":" STR(CURRENT_INDEX) ",\"last_index\":" STR(LAST_INDEX) ","
      "\"bundle_hash\":\"" TRYTES_81_2 "\","
      "\"trunk_transaction_hash\":\"" TRYTES_81_2 "\","
      "\"branch_transaction_hash\":\"" TRYTES_81_1 "\","
      "\"tag\":\"" TEST_TAG "\","
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
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TEST_TAG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
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

void test_send_mam_message_request_deserialize(void) {
  const char* json =
      "{\"seed\":\"" TRYTES_81_1
      "\",\"channel_ord\":" STR(TEST_CHANNEL_ORD) ",\"message\":\"" TEST_PAYLOAD "\",\"ch_mss_depth\":" STR(
          TEST_CH_DEPTH) ",\"ep_mss_depth\":" STR(TEST_EP_DEPTH) ",\"ntru_pk\":\"" TEST_NTRU_PK
                                                                 "\",\"psk\":\"" TRYTES_81_2 "\"}";
  ta_send_mam_req_t* req = send_mam_req_new();

  send_mam_req_deserialize(json, req);
  TEST_ASSERT_EQUAL_MEMORY(TRYTES_81_1, req->seed, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_INT(TEST_CHANNEL_ORD, req->channel_ord);
  TEST_ASSERT_EQUAL_STRING(TEST_PAYLOAD, req->message);
  TEST_ASSERT_EQUAL_INT(TEST_CH_DEPTH, req->ch_mss_depth);
  TEST_ASSERT_EQUAL_INT(TEST_EP_DEPTH, req->ep_mss_depth);
  TEST_ASSERT_EQUAL_MEMORY(TEST_NTRU_PK, req->ntru_pk, MAM_NTRU_PK_SIZE / 3);
  TEST_ASSERT_EQUAL_MEMORY(TRYTES_81_2, req->psk, NUM_TRYTES_HASH);

  send_mam_req_free(&req);
}

void test_send_mam_message_response_serialize(void) {
  const char* json = "{\"bundle_hash\":\"" TRYTES_81_1
                     "\","
                     "\"chid\":\"" TRYTES_81_2
                     "\","
                     "\"epid\":\"" TRYTES_81_3
                     "\","
                     "\"msg_id\":\"" TEST_MSG_ID
                     "\","
                     "\"announcement_bundle_hash\":\"" ADDRESS_1
                     "\","
                     "\"chid1\":\"" ADDRESS_2 "\"}";
  char* json_result;
  ta_send_mam_res_t* res = send_mam_res_new();

  send_mam_res_set_bundle_hash(res, (tryte_t*)TRYTES_81_1);
  send_mam_res_set_channel_id(res, (tryte_t*)TRYTES_81_2);
  send_mam_res_set_endpoint_id(res, (tryte_t*)TRYTES_81_3);
  send_mam_res_set_msg_id(res, (tryte_t*)TEST_MSG_ID);
  send_mam_res_set_announcement_bundle_hash(res, (tryte_t*)ADDRESS_1);
  send_mam_res_set_chid1(res, (tryte_t*)ADDRESS_2);

  send_mam_res_serialize(res, &json_result);
  TEST_ASSERT_EQUAL_STRING(json, json_result);

  free(json_result);
  send_mam_res_free(&res);
}

void test_send_mam_message_response_deserialize(void) {
  const char* json = "{\"bundle_hash\":\"" TRYTES_81_1
                     "\","
                     "\"chid\":\"" TRYTES_81_2
                     "\","
                     "\"epid\":\"" TRYTES_81_3
                     "\","
                     "\"msg_id\":\"" TEST_MSG_ID
                     "\","
                     "\"announcement_bundle_hash\":\"" ADDRESS_1
                     "\","
                     "\"chid1\":\"" ADDRESS_2 "\"}";
  ta_send_mam_res_t* res = send_mam_res_new();

  send_mam_res_deserialize(json, res);

  TEST_ASSERT_EQUAL_MEMORY(TRYTES_81_1, res->bundle_hash, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(TRYTES_81_2, res->chid, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(TRYTES_81_3, res->epid, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(TEST_MSG_ID, res->msg_id, MAM_MSG_ID_SIZE / 3);
  TEST_ASSERT_EQUAL_MEMORY(ADDRESS_1, res->announcement_bundle_hash, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(ADDRESS_2, res->chid1, NUM_TRYTES_HASH);
  send_mam_res_free(&res);
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
#ifdef MQTT_ENABLE
void test_mqtt_device_id_deserialize(void) {
  const char* json = "{\"device_id\":\"" DEVICE_ID "\", \"trytes\":[\"" TRYTES_2673_1 "\",\"" TRYTES_2673_2 "\"]}";
  const int id_len = 32;
  char device_id[ID_LEN + 1] = {0};
  TEST_ASSERT_EQUAL_INT(SC_OK, mqtt_device_id_deserialize(json, device_id));

  TEST_ASSERT_EQUAL_STRING(device_id, DEVICE_ID);
}

void test_mqtt_tag_req_deserialize(void) {
  const char* json = "{\"device_id\":\"" DEVICE_ID "\", \"tag\":\"" TEST_TAG "\"}";
  char tag[NUM_TRYTES_TAG + 1] = {0};
  TEST_ASSERT_EQUAL_INT(SC_OK, mqtt_tag_req_deserialize(json, tag));

  TEST_ASSERT_EQUAL_STRING(tag, TEST_TAG);
}

void test_mqtt_transaction_hash_req_deserialize(void) {
  const char* json = "{\"device_id\":\"" DEVICE_ID "\", \"hash\":\"" TRYTES_81_1 "\"}";
  char hash[NUM_TRYTES_HASH + 1];
  TEST_ASSERT_EQUAL_INT(SC_OK, mqtt_transaction_hash_req_deserialize(json, hash));

  TEST_ASSERT_EQUAL_STRING(hash, TRYTES_81_1);
}
#endif

void test_proxy_apis_command_req_deserialize(void) {
  const char* json =
      "{\"command\": \"" TEST_PROXY_API(addNeighbors) "\",\"uris\": [\"tcp://8.8.8.8:14265\",\"tcp://8.8.8.8:14265\"]}";
  char command[30];
  TEST_ASSERT_EQUAL_INT(SC_OK, proxy_apis_command_req_deserialize(json, command));

  TEST_ASSERT_EQUAL_STRING(command, TEST_PROXY_API(addNeighbors));
}

void test_get_iri_status_milestone_deserialize(void) {
  const char* json =
      "{\"appName\": \"IRI\",\"appVersion\": \"1.7.0-RELEASE\",\"jreAvailableProcessors\": 8,\"jreFreeMemory\": "
      "2115085674,\"jreVersion\": \"1.8.0_191\",\"jreMaxMemory\": 20997734400,\"jreTotalMemory\": "
      "4860129502,\"latestMilestone\": "
      "\"CUOENIPTRCNECMVOXSWKOONGZJICAPH9FIG9F9KYXF9VYXFUKTNDCCLLWRZNUHZIGLJZFWPOVCIZA9999\",\"latestMilestoneIndex\": "
      "1050373,\"latestSolidSubtangleMilestone\": "
      "\"999ENIPTRCNECMVOXSWKOONGZJICAPH9FIG9F9KYXF9VYXFUKTNDCCLLWRZNUHZIGLJZFWPOVCIZA9999\", "
      "\"latestSolidSubtangleMilestoneIndex\": 1050373, \"milestoneStartIndex\": 1050101, "
      "\"lastSnapshottedMilestoneIndex\": 1039138, \"neighbors\": 7, \"packetsQueueSize\": 0, \"time\": 1554970558971, "
      "\"tips\": 9018, \"transactionsToRequest\": 0, \"features\": [  \"snapshotPruning\",  \"dnsRefresher\",  "
      "\"tipSolidification\" ],\"coordinatorAddress\": "
      "\"EQSAUZXULTTYZCLNJNTXQTQHOMOFZERHTCGTXOLTVAHKSA9OGAZDEKECURBRIXIJWNPFCQIOVFVVXJVD9\",\"duration\": 0}";
  const char latestMilestone[] = "CUOENIPTRCNECMVOXSWKOONGZJICAPH9FIG9F9KYXF9VYXFUKTNDCCLLWRZNUHZIGLJZFWPOVCIZA9999";
  const char latestSolidSubtangleMilestone[] =
      "999ENIPTRCNECMVOXSWKOONGZJICAPH9FIG9F9KYXF9VYXFUKTNDCCLLWRZNUHZIGLJZFWPOVCIZA9999";
  char deserialize_latestMilestone[82], deserialize_latestSolidSubtangleMilestone[82];

  TEST_ASSERT_EQUAL_INT(SC_OK, get_iri_status_milestone_deserialize(json, deserialize_latestMilestone,
                                                                    deserialize_latestSolidSubtangleMilestone));
  TEST_ASSERT_EQUAL_STRING(latestMilestone, deserialize_latestMilestone);
  TEST_ASSERT_EQUAL_STRING(latestSolidSubtangleMilestone, deserialize_latestSolidSubtangleMilestone);
}

void test_get_iri_status_res_serialize(void) {
  const char* json = "{\"status\":false,\"status_code\":\"SC_CORE_IRI_UNSYNC\"}";
  char* json_result = NULL;
  TEST_ASSERT_EQUAL_STRING(SC_OK, get_iri_status_res_serialize(SC_CORE_IRI_UNSYNC, &json_result));
  TEST_ASSERT_EQUAL_STRING(json, json_result);

  free(json_result);
}

int main(void) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  serializer_logger_init();
  RUN_TEST(test_serialize_ta_generate_address);
  RUN_TEST(test_deserialize_ta_send_transfer_zero);
  RUN_TEST(test_deserialize_ta_send_transfer_value);
  RUN_TEST(test_serialize_ta_find_transaction_objects);
  RUN_TEST(test_serialize_ta_find_transactions_by_tag);
  RUN_TEST(test_serialize_ta_find_transactions_obj_by_tag);
  RUN_TEST(test_send_mam_message_response_serialize);
  RUN_TEST(test_send_mam_message_response_deserialize);
  RUN_TEST(test_send_mam_message_request_deserialize);
  RUN_TEST(test_deserialize_ta_send_trytes_req);
  RUN_TEST(test_serialize_ta_send_trytes_res);
#ifdef MQTT_ENABLE
  RUN_TEST(test_mqtt_device_id_deserialize);
  RUN_TEST(test_mqtt_tag_req_deserialize);
  RUN_TEST(test_mqtt_transaction_hash_req_deserialize);
#endif
  RUN_TEST(test_proxy_apis_command_req_deserialize);
  RUN_TEST(test_get_iri_status_milestone_deserialize);
  RUN_TEST(test_get_iri_status_res_serialize);
  serializer_logger_release();
  return UNITY_END();
}
