#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "./common_core.h"
#include "iota_api_mock.hh"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::ElementsAreArray;

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
  EXPECT_THAT(res->tips->hash, ElementsAreArray(hash));
  hash243_stack_pop(&res->tips);
  flex_trits_from_trytes(hash, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  EXPECT_THAT(res->tips->hash, ElementsAreArray(hash));
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
    EXPECT_THAT(s_iter->hash, ElementsAreArray(hash));
  }
  ta_get_tips_res_free(&res);
}

TEST(FindTxnTest, TxnHashTest) {
  ta_find_transactions_req_t* req = ta_find_transactions_req_new();
  ta_find_transactions_res_t* res = ta_find_transactions_res_new();
  flex_trit_t hash[FLEX_TRIT_SIZE_81];
  flex_trits_from_trytes(hash, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);

  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(ta_find_transactions_by_tag(&service, req, res), 0);
  hash243_queue_entry_t* q_iter = NULL;
  CDL_FOREACH(res->hashes, q_iter) {
    flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
    flex_trits_from_trytes(hash, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1,
                           NUM_TRYTES_HASH, NUM_TRYTES_HASH);
    EXPECT_THAT(q_iter->hash, ElementsAreArray(hash));
  }
  ta_find_transactions_req_free(&req);
  ta_find_transactions_res_free(&res);
}

TEST(GenAdressTest, GetNewAddressTest) {
  ta_generate_address_res_t* res = ta_generate_address_res_new();

  EXPECT_CALL(APIMockObj, iota_client_get_new_address(_, _, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(ta_generate_address(&service, res), 0);
  hash243_queue_entry_t* q_iter = NULL;
  CDL_FOREACH(res->addresses, q_iter) {
    flex_trit_t hash[FLEX_TRIT_SIZE_243] = {};
    flex_trits_from_trytes(hash, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1,
                           NUM_TRYTES_HASH, NUM_TRYTES_HASH);
    EXPECT_THAT(q_iter->hash, ElementsAreArray(hash));
  }
  ta_generate_address_res_free(&res);
}

TEST(GetTxnMsgTest, GetTrytesTest) {
  ta_get_transaction_msg_req_t* req = ta_get_transaction_msg_req_new();
  ta_get_transaction_msg_res_t* res = ta_get_transaction_msg_res_new();

  EXPECT_CALL(APIMockObj, iota_client_get_trytes(_, _, _)).Times(AtLeast(1));

  EXPECT_EQ(ta_get_transaction_msg(&service, req, res), 0);
  flex_trit_t hash[FLEX_TRIT_SIZE_6561];
  flex_trits_from_trytes(hash, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);

  EXPECT_THAT(res->msg, ElementsAreArray(hash));
  ta_get_transaction_msg_req_free(&req);
  ta_get_transaction_msg_res_free(&res);
}

int main(int argc, char** argv) {
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
