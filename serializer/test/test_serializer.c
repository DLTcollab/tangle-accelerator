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

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_deserialize_ta_get_tips);
  RUN_TEST(test_serialize_ta_get_tips);

  return UNITY_END();
}
