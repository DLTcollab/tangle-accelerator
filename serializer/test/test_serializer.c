#include "test_serializer.h"

void test_deserialize_ta_get_tips(void) {
  const char* json = "{\"command\":\"get_tips\",\"opt\":1}";

  ta_get_tips_req_t* req = ta_get_tips_req_new();
  ta_get_tips_req_deserialize(json, req);

  TEST_ASSERT_EQUAL_INT(1, req->opt);
  ta_get_tips_req_free(req);
}

void test_serialize_ta_get_tips(void) {
  const char* json = "{\"tips\":[\"" TRYRES_81_1 "\",\"" TRYRES_81_2 "\"]}";
  char* json_result;
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  res = flex_hash_array_append(res, TRYRES_81_1);
  res = flex_hash_array_append(res, TRYRES_81_2);

  ta_get_tips_res_serialize(&json_result, res);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_get_tips_res_free(res);
}

void test_serialize_ta_generate_address(void) {
  const char* json = "{\"address\":[\"" TRYRES_81_1 "\",\"" TRYRES_81_2 "\"]}";
  char* json_result;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  res = flex_hash_array_append(res, TRYRES_81_1);
  res = flex_hash_array_append(res, TRYRES_81_2);

  ta_generate_address_res_serialize(&json_result, res);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_generate_address_res_free(res);
}

void test_deserialize_ta_send_transfer(void) {
  const char* json =
      "{\"command\":\"send_transfer\",\"value\":100,"
      "\"message\":\"" TAG_MSG "\",\"tag\":\"" TAG_MSG
      "\","
      "\"address\":\"" TRYRES_81_1 "\"}";

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_req_deserialize(json, req);

  TEST_ASSERT_EQUAL_INT(100, req->value);
  char trytes_out[TAG_MSG_LEN + 1];
  flex_trits_to_trytes((tryte_t*)trytes_out, TAG_MSG_LEN, req->tag->trits,
                       TAG_MSG_LEN * 3, TAG_MSG_LEN * 3);
  trytes_out[TAG_MSG_LEN] = '\0';
  TEST_ASSERT_EQUAL_STRING(TAG_MSG, trytes_out);

  flex_trits_to_trytes((tryte_t*)trytes_out, TAG_MSG_LEN, req->message->trits,
                       TAG_MSG_LEN * 3, TAG_MSG_LEN * 3);
  trytes_out[TAG_MSG_LEN] = '\0';
  TEST_ASSERT_EQUAL_STRING(TAG_MSG, trytes_out);

  char addr_trytes[82];
  flex_trits_to_trytes((tryte_t*)addr_trytes, 81, req->address->trits, 243,
                       243);
  addr_trytes[81] = '\0';
  TEST_ASSERT_EQUAL_STRING(TRYRES_81_1, addr_trytes);

  ta_send_transfer_req_free(req);
}

void test_serialize_ta_send_transfer(void) {
  const char* json = "{\"bundle\":\"" TRYRES_2673_1 "\"}";
  char* json_result;
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  res->bundle = trit_array_new_from_trytes((const tryte_t*)TRYRES_2673_1);

  ta_send_transfer_res_serialize(&json_result, res);
  TEST_ASSERT_EQUAL_STRING(json, json_result);
  ta_send_transfer_res_free(res);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_deserialize_ta_get_tips);
  RUN_TEST(test_serialize_ta_get_tips);
  RUN_TEST(test_serialize_ta_generate_address);
  RUN_TEST(test_deserialize_ta_send_transfer);
  RUN_TEST(test_serialize_ta_send_transfer);

  return UNITY_END();
}
