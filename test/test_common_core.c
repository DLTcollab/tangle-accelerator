#include "test_common_core.h"

void test_insert_to_trytes(void) {
  char trytes_out[STR_ANS_LEN + 1];
  size_t start = 0;
  trit_array_p trytes = trit_array_new_from_trytes((const tryte_t*)STR_A);
  trit_array_p from_trytes = trit_array_new_from_trytes((const tryte_t*)STR_B);

  trytes = insert_to_trytes(start, trytes, from_trytes);

  flex_trits_to_trytes(trytes_out, STR_ANS_LEN, trytes->trits, STR_ANS_LEN * 3,
                       STR_ANS_LEN * 3);
  trytes_out[STR_ANS_LEN] = '\0';
  TEST_ASSERT_EQUAL_STRING(STR_ANS, trytes_out);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_insert_to_trytes);

  return UNITY_END();
}
