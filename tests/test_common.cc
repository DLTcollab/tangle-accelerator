#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "accelerator/common_core.h"
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
  const char req[FLEX_TRIT_SIZE_81] = {};
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

TEST(GetTxnObjTest, GetTrytesTest) {
  const char req[FLEX_TRIT_SIZE_243] = {};
  ta_get_transaction_object_res_t* res = ta_get_transaction_object_res_new();

  EXPECT_CALL(APIMockObj, iota_client_get_trytes(_, _, _)).Times(AtLeast(0));

  EXPECT_EQ(ta_get_transaction_object(&service, req, res), 0);
  flex_trit_t hash[FLEX_TRIT_SIZE_6561];
  flex_trits_from_trytes(hash, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);

  EXPECT_THAT(res->txn->data.signature_or_message, ElementsAreArray(hash));
  ta_get_transaction_object_res_free(&res);
}

TEST(FindTxnObjTest, TxnObjTest) {
  const char req[NUM_TRYTES_TAG] = {};
  const iota_transaction_t* txn = NULL;
  ta_find_transactions_obj_res_t* res = ta_find_transactions_obj_res_new();
  flex_trit_t msg[FLEX_TRIT_SIZE_6561];
  flex_trits_from_trytes(msg, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);

  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(APIMockObj, iota_client_get_trytes(_, _, _)).Times(AtLeast(0));

  EXPECT_EQ(ta_find_transactions_obj_by_tag(&service, req, res), 0);
  for (txn = (const iota_transaction_t*)utarray_front(res->txn_obj);
       txn != NULL;
       txn = (const iota_transaction_t*)utarray_next(res->txn_obj, txn)) {
    flex_trit_t txn_msg[FLEX_TRIT_SIZE_6561];
    flex_trits_from_trytes(txn_msg, NUM_TRITS_SIGNATURE,
                           (const tryte_t*)transaction_message(txn),
                           NUM_TRYTES_SIGNATURE, NUM_TRYTES_SIGNATURE);
    EXPECT_THAT(txn_msg, ElementsAreArray(msg));
  }
  ta_find_transactions_obj_res_free(&res);
}

TEST(SendTransferTest, SendTransferTest) {
  flex_trit_t* txn_hash;
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561], tag_trits[FLEX_TRIT_SIZE_81];

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  req->value = 0;
  hash243_queue_push(&req->address, TRITS_81_1);
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  hash81_queue_push(&req->tag, tag_trits);
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  req->msg_len = NUM_TRITS_SIGNATURE;
  flex_trits_slice(req->message, req->msg_len, msg_trits, req->msg_len, 0,
                   req->msg_len);
  EXPECT_CALL(APIMockObj, iota_client_send_trytes(_, _, _, _, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(ta_send_transfer(&service, req, res), 0);
  txn_hash = hash243_queue_peek(res->hash);
  EXPECT_TRUE(
      std::is_permutation(txn_hash, txn_hash + FLEX_TRIT_SIZE_81, TRITS_81_1));

  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
}

int main(int argc, char** argv) {
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
