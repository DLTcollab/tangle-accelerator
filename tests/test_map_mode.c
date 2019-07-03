/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "map/mode.h"
#include "test_define.h"

#define CHID "UANFAVTSAXZMYUWRECNAOJDAQVTTORVGJCCISMZYAFFU9EYLBMZKEJ9VNXVFFGUTCHONEYVWVUTBTDJLO"
#define NEW_CHID "ONMTPDICUWBGEGODWKGBGMLNAZFXNHCJITSSTBTGMXCXBXJFBPOPXFPOJTXKOOSAJOZAYANZZBFKYHJ9N"
#define EPID "KI99YKKLFALYRUVRXKKRJCPVFISPMNCQQSMB9BGUWIHZTYFQOBZWYSVRNKVFJLSPPLPSFNBNREJWOR99U"

void test_channel_create(void) {
  mam_api_t mam;
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  map_channel_create(&mam, channel_id, 1);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  TEST_ASSERT_EQUAL_STRING(CHID, channel_id);

  mam_api_destroy(&mam);
}

void test_announce_channel(void) {
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  map_channel_create(&mam, channel_id, 1);
  map_announce_channel(&mam, channel_id, bundle, msg_id, channel_id);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  TEST_ASSERT_EQUAL_STRING(NEW_CHID, channel_id);

  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
}

void test_announce_endpoint(void) {
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);

  map_channel_create(&mam, channel_id, 1);
  // Channel_id is actually the new endpoint id
  map_announce_endpoint(&mam, channel_id, bundle, msg_id, channel_id);
  channel_id[MAM_CHANNEL_ID_TRYTE_SIZE] = '\0';
  TEST_ASSERT_EQUAL_STRING(EPID, channel_id);

  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
}

void test_write_message(void) {
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);
  tryte_t channel_id[MAM_CHANNEL_ID_TRYTE_SIZE + 1];
  trit_t msg_id[MAM_MSG_ID_SIZE];
  mam_api_init(&mam, (tryte_t *)TRYTES_81_1);
  retcode_t ret = RC_ERROR;

  map_channel_create(&mam, channel_id, 1);
  ret = map_write_header_on_channel(&mam, channel_id, bundle, msg_id);
  TEST_ASSERT_EQUAL(RC_OK, ret);

  ret = map_write_packet(&mam, bundle, TEST_PAYLOAD, msg_id, true);
  TEST_ASSERT_EQUAL(RC_OK, ret);

  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
}

void test_bundle_read(void) {
  retcode_t ret;
  char *payload = NULL;
  mam_api_t mam;
  bundle_transactions_t *bundle = NULL;
  bundle_transactions_new(&bundle);

  ret = mam_api_init(&mam, (tryte_t *)SEED);
  TEST_ASSERT_EQUAL(RC_OK, ret);

  flex_trit_t chid_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(chid_trits, NUM_TRITS_HASH, (const tryte_t *)CHID_BUNDLE, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  mam_api_add_trusted_channel_pk(&mam, chid_trits);

  flex_trit_t hash[NUM_TRITS_SERIALIZED_TRANSACTION];
  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t const *)TEST_MAM_TRANSACTION_TRYTES_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  iota_transaction_t *txn = transaction_deserialize(hash, false);
  bundle_transactions_add(bundle, txn);
  transaction_free(txn);

  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t const *)TEST_MAM_TRANSACTION_TRYTES_2,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  txn = transaction_deserialize(hash, false);
  bundle_transactions_add(bundle, txn);
  transaction_free(txn);

  flex_trits_from_trytes(hash, NUM_TRITS_SERIALIZED_TRANSACTION, (tryte_t const *)TEST_MAM_TRANSACTION_TRYTES_3,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  txn = transaction_deserialize(hash, false);
  bundle_transactions_add(bundle, txn);

  ret = map_api_bundle_read(&mam, bundle, &payload);
  TEST_ASSERT_EQUAL(RC_OK, ret);

  TEST_ASSERT_EQUAL_STRING(TEST_PAYLOAD, payload);
  transaction_free(txn);
  bundle_transactions_free(&bundle);
  mam_api_destroy(&mam);
  free(payload);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_channel_create);
  RUN_TEST(test_announce_channel);
  RUN_TEST(test_announce_endpoint);
  RUN_TEST(test_write_message);
  RUN_TEST(test_bundle_read);
  return UNITY_END();
}
