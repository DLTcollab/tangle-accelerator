/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include "accelerator/common_core.h"
#include "iota_api_mock.hh"

using ::testing::AtLeast;
using ::testing::ElementsAreArray;
using ::testing::_;

APIMock APIMockObj;
iota_config_t tangle;
iota_client_service_t service;

TEST(GetTxnToApproveTest, TrunkBranchHashTest) {
  ta_get_tips_res_t* res = ta_get_tips_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243], hash_trits_2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);

  EXPECT_CALL(APIMockObj, iota_client_get_transactions_to_approve(_, _, _)).Times(AtLeast(1));

  EXPECT_EQ(cclient_get_txn_to_approve(&service, 3, res), 0);
  EXPECT_FALSE(memcmp(res->tips->hash, hash_trits_1, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  hash243_stack_pop(&res->tips);
  EXPECT_FALSE(memcmp(res->tips->hash, hash_trits_2, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  ta_get_tips_res_free(&res);
}

TEST(GenAdressTest, GetNewAddressTest) {
  tangle.seed = SEED;
  hash243_queue_entry_t* q_iter = NULL;
  ta_generate_address_res_t* res = ta_generate_address_res_new();
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);

  EXPECT_CALL(APIMockObj, iota_client_get_new_address(_, _, _, _)).Times(AtLeast(1));

  EXPECT_EQ(ta_generate_address(&tangle, &service, res), 0);
  CDL_FOREACH(res->addresses, q_iter) {
    EXPECT_FALSE(memcmp(q_iter->hash, hash_trits_1, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  }
  ta_generate_address_res_free(&res);
}

TEST(GetTxnObjTest, GetTrytesTest) {
  flex_trit_t* txn_msg = NULL;
  ta_find_transaction_objects_req_t* req = ta_find_transaction_objects_req_new();
  transaction_array_t* res = transaction_array_new();

  flex_trit_t tx_trits[NUM_TRITS_SERIALIZED_TRANSACTION];
  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_2,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  iota_transaction_t* expected_txn = transaction_deserialize(tx_trits, true);
  hash243_queue_push(&req->hashes, transaction_hash(expected_txn));

  EXPECT_CALL(APIMockObj, iota_client_get_transaction_objects(_, _, _)).Times(AtLeast(0));
  EXPECT_EQ(ta_find_transaction_objects(&service, req, res), 0);

  iota_transaction_t* txn = transaction_array_at(res, 0);
  EXPECT_EQ(
      memcmp(transaction_address(expected_txn), transaction_address(txn), (sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243)),
      0);
  EXPECT_EQ(memcmp(transaction_obsolete_tag(expected_txn), transaction_obsolete_tag(txn),
                   (sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243)),
            0);
  EXPECT_EQ(
      memcmp(transaction_message(expected_txn), transaction_message(txn), (sizeof(flex_trit_t) * FLEX_TRIT_SIZE_6561)),
      0);
  EXPECT_EQ(memcmp(transaction_tag(expected_txn), transaction_tag(txn), (sizeof(flex_trit_t) * FLEX_TRIT_SIZE_81)), 0);
  EXPECT_EQ(transaction_value(expected_txn), transaction_value(txn));
  EXPECT_EQ(transaction_current_index(expected_txn), transaction_current_index(txn));

  ta_find_transaction_objects_req_free(&req);
  transaction_array_free(res);
  transaction_free(expected_txn);
}

TEST(SendTransferTest, SendTransferTest) {
  flex_trit_t* txn_hash;
  flex_trit_t msg_trits[FLEX_TRIT_SIZE_6561], tag_trits[FLEX_TRIT_SIZE_81], hash_trits_1[FLEX_TRIT_SIZE_243];

  ta_send_transfer_req_t* req = ta_send_transfer_req_new();
  ta_send_transfer_res_t* res = ta_send_transfer_res_new();
  req->value = 0;
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(&req->address, hash_trits_1);
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)TAG_MSG, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  hash81_queue_push(&req->tag, tag_trits);
  flex_trits_from_trytes(msg_trits, NUM_TRITS_SIGNATURE, (const tryte_t*)TRYTES_2187_1, NUM_TRYTES_SIGNATURE,
                         NUM_TRYTES_SIGNATURE);
  req->msg_len = NUM_TRITS_SIGNATURE;
  flex_trits_slice(req->message, req->msg_len, msg_trits, req->msg_len, 0, req->msg_len);

  EXPECT_CALL(APIMockObj, ta_send_trytes(_, _, _)).Times(AtLeast(1));
  EXPECT_CALL(APIMockObj, iota_client_find_transactions(_, _, _)).Times(AtLeast(1));

  EXPECT_EQ(ta_send_transfer(&tangle, &service, req, res), 0);
  txn_hash = hash243_queue_peek(res->hash);
  EXPECT_FALSE(memcmp(txn_hash, hash_trits_1, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));

  ta_send_transfer_req_free(&req);
  ta_send_transfer_res_free(&res);
}

TEST(GetBundleTest, RetreiveBundleTest) {
  bundle_transactions_t* bundle = NULL;
  bundle_transactions_new(&bundle);
  iota_transaction_t* tx_iter = NULL;
  flex_trit_t* tx = NULL;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

  EXPECT_CALL(APIMockObj, iota_client_find_transaction_objects(_, _, _)).Times(AtLeast(1));
  EXPECT_EQ(ta_get_bundle(&service, (const tryte_t*)TRYTES_81_1, bundle), 0);

  BUNDLE_FOREACH(bundle, tx_iter) {
    tx = transaction_serialize(tx_iter);
    EXPECT_FALSE(memcmp(tx, tx_trits, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_8019));
  }

  free(tx);
  bundle_transactions_free(&bundle);
}

TEST(SendTrytesTest, SendTrytesTest) {
  size_t trits_count;
  hash8019_array_p trytes = hash8019_array_new();
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  tryte_t trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {};

  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  hash_array_push(trytes, tx_trits);

  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_2,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  hash_array_push(trytes, tx_trits);

  EXPECT_EQ(ta_send_trytes(&tangle, &service, trytes), SC_OK);

  trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_SERIALIZED_TRANSACTION, hash_array_at(trytes, 0),
                                     NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
  trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';
  EXPECT_EQ(NUM_TRITS_SERIALIZED_TRANSACTION, trits_count);
  EXPECT_STREQ(TRYTES_2673_1, (char*)trytes_out);

  trits_count = flex_trits_to_trytes(trytes_out, NUM_TRYTES_SERIALIZED_TRANSACTION, hash_array_at(trytes, 1),
                                     NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
  trytes_out[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';
  EXPECT_EQ(NUM_TRITS_SERIALIZED_TRANSACTION, trits_count);
  EXPECT_STREQ(TRYTES_2673_2, (char*)trytes_out);

  hash_array_free(trytes);
}

int main(int argc, char** argv) {
  // GTest manage to cleanup after testing, so only need to initialize here
  cache_init(true, REDIS_HOST, REDIS_PORT);
  ::testing::GTEST_FLAG(throw_on_failure) = true;
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
