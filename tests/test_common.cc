#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <unity/unity.h>
#include "./common_core.h"
#include "iota_api_mock.hh"

using ::testing::_;
using ::testing::AtLeast;

APIMock APIMockObj;
iota_client_service_t service;

TEST(GetTxnToApproveTest, TrunkBranchHashTest) {
  ta_get_tips_res_t* res = ta_get_tips_res_new();

  EXPECT_CALL(APIMockObj, iota_client_get_transactions_to_approve(_, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(cclient_get_txn_to_approve(&service, res), 0);
  flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
  flex_trits_from_trytes(hash, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(hash, hash243_stack_peek(res->tips),
                           FLEX_TRIT_SIZE_243);
  hash243_stack_pop(&res->tips);
  flex_trits_from_trytes(hash, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  TEST_ASSERT_EQUAL_MEMORY(hash, hash243_stack_peek(res->tips),
                           FLEX_TRIT_SIZE_243);
  ta_get_tips_res_free(&res);
}

TEST(GetTipsTest, TipsHashTest) {
  ta_get_tips_res_t* res = ta_get_tips_res_new();

  EXPECT_CALL(APIMockObj, iota_client_get_tips(_, _)).Times(AtLeast(1));

  EXPECT_EQ(cclient_get_tips(&service, res), 0);
  hash243_stack_entry_t* s_iter = NULL;
  LL_FOREACH(res->tips, s_iter) {
    flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
    flex_trits_from_trytes(hash, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1,
                           NUM_TRYTES_HASH, NUM_TRYTES_HASH);
    TEST_ASSERT_EQUAL_MEMORY(hash, s_iter->hash, FLEX_TRIT_SIZE_243);
  }
  ta_get_tips_res_free(&res);
}

int main(int argc, char** argv) {
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
