#include "test_serializer.h"

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
      "{\"command\":\"send_transfer\",\"value\":100,"
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

void test_serialize_ta_get_transaction_msg(void) {
  const char* json = "{\"message\":\"" TRYTES_2187_1 "\"}";
  char* json_result;
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561];
  ta_get_transaction_msg_res_t* res = ta_get_transaction_msg_res_new();

  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  memcpy(res->msg, msg_trits, FLEX_TRIT_SIZE_6561);

  ta_get_transaction_msg_res_serialize(&json_result, res);

  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_get_transaction_msg_res_free(&res);
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
  RUN_TEST(test_serialize_ta_get_transaction_msg);
  RUN_TEST(test_serialize_ta_find_transactions_by_tag);

  return UNITY_END();
}
