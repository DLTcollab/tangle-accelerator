/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "test_define.h"
#include "utils/pow.h"

void test_pow_flex(void) {
  int mwm = 9;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trit_t* nonce_trits;
  flex_trit_t null_trits[mwm];

  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

  nonce_trits = ta_pow_flex(tx_trits, mwm);

  /* Validation */
  flex_trits_slice(null_trits, mwm, nonce_trits, NUM_TRITS_NONCE, NUM_TRITS_NONCE - mwm, mwm);
  TEST_ASSERT_TRUE(flex_trits_are_null(null_trits, mwm / 3));

  free(nonce_trits);
}

void test_pow_bundle(void) {
  bundle_transactions_t* bundle = NULL;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trit_t hash_trits_2[FLEX_TRIT_SIZE_243];
  iota_transaction_t tx;

  bundle_transactions_new(&bundle);
  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  transaction_deserialize_from_trits(&tx, tx_trits, false);
  bundle_transactions_add(bundle, &tx);

  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  ta_pow(bundle, hash_trits_1, hash_trits_2, 9);

  // bundle to trytes
  iota_transaction_t* tx_iter = NULL;
  BUNDLE_FOREACH(bundle, tx_iter) {
    TEST_ASSERT_FALSE(memcmp(transaction_trunk(tx_iter), hash_trits_1, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
    TEST_ASSERT_FALSE(memcmp(transaction_branch(tx_iter), hash_trits_2, sizeof(flex_trit_t) * FLEX_TRIT_SIZE_243));
  }

  bundle_transactions_free(&bundle);
}

int main(void) {
  UNITY_BEGIN();
  pow_init();
  RUN_TEST(test_pow_flex);
  RUN_TEST(test_pow_bundle);
  pow_destroy();
  return UNITY_END();
}
