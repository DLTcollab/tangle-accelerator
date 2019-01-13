#include "test_serializer.h"
#include <inttypes.h>

void test_serialize_ta_get_tips(void) {
  const char* json = "{\"tips\":[\"" TRYTES_81_1 "\",\"" TRYTES_81_2 "\"]}";
  char* json_result;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  hash243_stack_push(&res->tips, TRITS_81_2);
  hash243_stack_push(&res->tips, TRITS_81_1);

  ta_get_tips_res_serialize(&json_result, res);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_get_tips_res_free(&res);
  free(json_result);
}

void test_serialize_ta_generate_address(void) {
  const char* json = "{\"address\":[\"" TRYTES_81_1 "\",\"" TRYTES_81_2 "\"]}";
  char* json_result;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  hash243_queue_push(&res->addresses, TRITS_81_1);
  hash243_queue_push(&res->addresses, TRITS_81_2);

  ta_generate_address_res_serialize(&json_result, res);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_generate_address_res_free(&res);
  free(json_result);
}

void test_deserialize_ta_send_transfer(void) {
  const char* json =
      "{\"value\":100,"
      "\"message\":\"" TAG_MSG "\",\"tag\":\"" TAG_MSG
      "\","
      "\"address\":\"" TRYTES_81_1 "\"}";

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  flex_trit_t tag_msg_trits[TAG_MSG_LEN * 3];
  flex_trit_t addr_trits[243];

  ta_send_transfer_req_deserialize(json, req);

  TEST_ASSERT_EQUAL_INT(100, req->value);
  flex_trits_from_trytes(tag_msg_trits, TAG_MSG_LEN * 3,
                         (const tryte_t*)TAG_MSG, TAG_MSG_LEN, TAG_MSG_LEN);
  TEST_ASSERT_EQUAL_MEMORY(tag_msg_trits, req->tag->hash, TAG_MSG_LEN);
  TEST_ASSERT_EQUAL_MEMORY(tag_msg_trits, req->message, TAG_MSG_LEN);

  flex_trits_from_trytes(addr_trits, 243, (const tryte_t*)TRYTES_81_1, 81, 81);
  TEST_ASSERT_EQUAL_MEMORY(TRITS_81_1, req->address->hash, 81);

  ta_send_transfer_req_free(&req);
}

void test_serialize_ta_send_transfer(void) {
  const char* json = "{\"hash\":\"" TRYTES_81_1 "\"}";
  char* json_result;
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();

  hash243_queue_push(&res->hash, TRITS_81_1);
  ta_send_transfer_res_serialize(&json_result, res);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_send_transfer_res_free(&res);
  free(json_result);
}

void test_serialize_ta_get_transaction_object(void) {
  const char* json = 
      "{\"hash\":\"" TRYTES_81_1 "\","
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
      "\"nonce\":\"" NONCE "\"}";
  char* json_result;
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561], tag_trits[FLEX_TRIT_SIZE_81];
  ta_get_transaction_object_res_t* res = ta_get_transaction_object_res_new();
  res->txn = transaction_new();

  // mask must be set in order to use the helper function
  res->txn->loaded_columns_mask |=
      (MASK_ESSENCE | MASK_ATTACHMENT | MASK_CONSENSUS | MASK_DATA);

  // set transaction hash
  transaction_set_hash(res->txn, TRITS_81_1);

  // set message
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  transaction_set_signature(res->txn, msg_trits);

  // set address
  transaction_set_address(res->txn, TRITS_81_1);
  // set value
  transaction_set_value(res->txn, VALUE);

  // set obsolete_tag
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  transaction_set_obsolete_tag(res->txn, tag_trits);

  // set timestamp
  transaction_set_timestamp(res->txn, TIMESTAMP);
  // set current_index
  transaction_set_current_index(res->txn, CURRENT_INDEX);
  // set last_index
  transaction_set_last_index(res->txn, LAST_INDEX);
  // set bundle_hash
  transaction_set_bundle(res->txn, TRITS_81_2);
  // set trunk
  transaction_set_trunk(res->txn, TRITS_81_2);
  // set branch
  transaction_set_branch(res->txn, TRITS_81_1);
  // set tag
  transaction_set_tag(res->txn, tag_trits);
  // set attachment_timestamp
  transaction_set_attachment_timestamp(res->txn, TIMESTAMP);
  // set attachment_timestamp_lower_bound
  transaction_set_attachment_timestamp_lower(res->txn, TIMESTAMP);
  // set attachment_timestamp_upper_bound
  transaction_set_attachment_timestamp_upper(res->txn, TIMESTAMP);
  // set nonce
  flex_trits_from_trytes(tag_trits, NUM_TRITS_NONCE, (const tryte_t*)NONCE,
                         NUM_TRYTES_NONCE, NUM_TRYTES_NONCE);
  transaction_set_nonce(res->txn, tag_trits);

  ta_get_transaction_object_res_serialize(&json_result, res);

  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_get_transaction_object_res_free(&res);
  free(json_result);
}

void test_serialize_ta_find_transactions_by_tag(void) {
  const char* json = "{\"hashes\":[\"" TRYTES_81_1 "\",\"" TRYTES_81_2 "\"]}";
  char* json_result;
  ta_find_transactions_res_t* res = ta_find_transactions_res_new();

  hash243_queue_push(&res->hashes, TRITS_81_1);
  hash243_queue_push(&res->hashes, TRITS_81_2);

  ta_find_transactions_res_serialize(&json_result, res);

  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_find_transactions_res_free(&res);
  free(json_result);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_serialize_ta_get_tips);
  RUN_TEST(test_serialize_ta_generate_address);
  RUN_TEST(test_deserialize_ta_send_transfer);
  RUN_TEST(test_serialize_ta_send_transfer);
  RUN_TEST(test_serialize_ta_get_transaction_object);
  RUN_TEST(test_serialize_ta_find_transactions_by_tag);
  return UNITY_END();
}
