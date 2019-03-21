#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "accelerator/common_core.h"
#include "iota_api_mock.hh"

using ::testing::_;
using ::testing::AtLeast;
using ::testing::ElementsAreArray;

APIMock APIMockObj;
iota_config_t config;
iota_client_service_t service;

TEST(GetTxnToApproveTest, TrunkBranchHashTest) {
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243],
      hash_trits_2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);

  EXPECT_CALL(APIMockObj, iota_client_get_transactions_to_approve(_, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(cclient_get_txn_to_approve(&service, 3, res), 0);
  EXPECT_FALSE(memcmp(res->tips->hash, hash_trits_1,
                      sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  hash243_stack_pop(&res->tips);
  EXPECT_FALSE(memcmp(res->tips->hash, hash_trits_2,
                      sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  ta_get_tips_res_free(&res);
}

TEST(GetTipsTest, TipsHashTest) {
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);

  EXPECT_CALL(APIMockObj, iota_client_get_tips(_, _)).Times(AtLeast(1));

  EXPECT_EQ(cclient_get_tips(&service, res), 0);
  hash243_stack_entry_t* s_iter = NULL;
  LL_FOREACH(res->tips, s_iter) {
    EXPECT_FALSE(memcmp(s_iter->hash, hash_trits_1,
                        sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  }
  ta_get_tips_res_free(&res);
}

TEST(FindTxnTest, TxnHashTest) {
  const char req[NUM_TRYTES_TAG] = {};
  ta_find_transactions_res_t* res = ta_find_transactions_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);

  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(ta_find_transactions_by_tag(&service, req, res), 0);
  hash243_queue_entry_t* q_iter = NULL;
  CDL_FOREACH(res->hashes, q_iter) {
    EXPECT_FALSE(memcmp(q_iter->hash, hash_trits_1,
                        sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  }
  ta_find_transactions_res_free(&res);
}

TEST(GenAdressTest, GetNewAddressTest) {
  config.seed = SEED;
  hash243_queue_entry_t* q_iter = NULL;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);

  EXPECT_CALL(APIMockObj, iota_client_get_new_address(_, _, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(ta_generate_address(&config, &service, res), 0);
  CDL_FOREACH(res->addresses, q_iter) {
    EXPECT_FALSE(memcmp(q_iter->hash, hash_trits_1,
                        sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  }
  ta_generate_address_res_free(&res);
}

TEST(GetTxnObjTest, GetTrytesTest) {
  const char req[FLEX_TRIT_SIZE_243] = {};
  flex_trit_t* txn_msg = NULL;
  ta_get_transaction_object_res_t* res = ta_get_transaction_object_res_new();
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561];
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);

  EXPECT_CALL(APIMockObj, iota_client_get_trytes(_, _, _)).Times(AtLeast(0));
  EXPECT_EQ(ta_get_transaction_object(&service, req, res), 0);
  txn_msg = transaction_message(res->txn);
  EXPECT_FALSE(
      memcmp(txn_msg, msg_trits, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_6561));
  ta_get_transaction_object_res_free(&res);
}

TEST(FindTxnObjTest, TxnObjTest) {
  const char req[NUM_TRYTES_TAG] = {};
  const iota_transaction_t* txn = NULL;
  flex_trit_t* txn_msg = NULL;
  ta_find_transactions_obj_res_t* res = ta_find_transactions_obj_res_new();
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561];
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);

  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _))
      .Times(AtLeast(1));
  EXPECT_CALL(APIMockObj, iota_client_get_trytes(_, _, _)).Times(AtLeast(0));

  EXPECT_EQ(ta_find_transactions_obj_by_tag(&service, req, res), 0);
  for (txn = (const iota_transaction_t*)utarray_front(res->txn_obj);
       txn != NULL;
       txn = (const iota_transaction_t*)utarray_next(res->txn_obj, txn)) {
    txn_msg = transaction_message(txn);
    EXPECT_FALSE(
        memcmp(txn_msg, msg_trits, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_6561));
  }
  ta_find_transactions_obj_res_free(&res);
}

TEST(SendTransferTest, SendTransferTest) {
  flex_trit_t* txn_hash;
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561], tag_trits[FLEX_TRIT_SIZE_81],
      hash_trits_1[FLEX_TRIT_SIZE_243];

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  req->value = 0;
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  hash243_queue_push(&req->address, hash_trits_1);
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  hash81_queue_push(&req->tag, tag_trits);
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE,
                         (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  req->msg_len = NUM_TRITS_SIGNATURE;
  flex_trits_slice(req->message, req->msg_len, msg_trits, req->msg_len, 0,
                   req->msg_len);

  EXPECT_CALL(APIMockObj, ta_send_trytes(_, _, _, _)).Times(AtLeast(1));
  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _))
      .Times(AtLeast(1));

  EXPECT_EQ(ta_send_transfer(&config, &service, req, res), 0);
  txn_hash = hash243_queue_peek(res->hash);
  EXPECT_FALSE(
      memcmp(txn_hash, hash_trits_1, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));

  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
}

TEST(GetBundleTest, RetreiveBundleTest) {
  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);
  iota_transaction_t* tx_iter = NULL;
  flex_trit_t* tx = NULL;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trits_from_trytes(
      tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
      NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

  EXPECT_CALL(APIMockObj, iota_client_find_transaction_objects(_, _, _))
      .Times(AtLeast(1));
  EXPECT_EQ(ta_get_bundle(&service, (const tryte_t*)TRYTES_81_1, bundle), 0);

  BUNDLE_FOREACH(bundle, tx_iter) {
    tx = transaction_serialize(tx_iter);
    EXPECT_FALSE(
        memcmp(tx, tx_trits, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_8019));
  }

  free(tx);
  bundle_transactions_free(&bundle);
}

int main(int argc, char** argv) {
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
