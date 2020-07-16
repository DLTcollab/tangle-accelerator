/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "debug.h"

#define DEBUG_LOGGER "ta_debug"

static logger_id_t logger_id;

void debug_logger_init() { logger_id = logger_helper_enable(DEBUG_LOGGER, LOGGER_DEBUG, true); }

int debug_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", DEBUG_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

void dump_bundle(bundle_transactions_t *bundle) {
  iota_transaction_t *curr_tx = NULL;
  BUNDLE_FOREACH(bundle, curr_tx) { dump_transaction_obj(curr_tx); }
}

void dump_transaction_obj(iota_transaction_t *tx_obj) {
  field_mask_t old_mask = {};
  memcpy(&old_mask, &tx_obj->loaded_columns_mask, sizeof(field_mask_t));
  memset(&tx_obj->loaded_columns_mask, 0xFF, sizeof(field_mask_t));

  ta_log_debug("==========Transaction Object==========\n");
  // address
  ta_log_debug("address: ");
  flex_trit_print(transaction_address(tx_obj), NUM_TRITS_ADDRESS);
  ta_log_debug("\n");

  ta_log_debug("value: %" PRId64 "\n", transaction_value(tx_obj));

  ta_log_debug("obsolete tag: ");
  flex_trit_print(transaction_obsolete_tag(tx_obj), NUM_TRITS_OBSOLETE_TAG);
  ta_log_debug("\n");

  ta_log_debug("timestamp: %" PRId64 "\n", transaction_timestamp(tx_obj));
  ta_log_debug("curr index: %" PRId64 " \nlast index: %" PRId64 "\n", transaction_current_index(tx_obj),
               transaction_last_index(tx_obj));

  ta_log_debug("bundle: ");
  flex_trit_print(transaction_bundle(tx_obj), NUM_TRITS_BUNDLE);
  ta_log_debug("\n");

  ta_log_debug("trunk: ");
  flex_trit_print(transaction_trunk(tx_obj), NUM_TRITS_TRUNK);
  ta_log_debug("\n");

  ta_log_debug("branch: ");
  flex_trit_print(transaction_branch(tx_obj), NUM_TRITS_BRANCH);
  ta_log_debug("\n");

  ta_log_debug("tag: ");
  flex_trit_print(transaction_tag(tx_obj), NUM_TRITS_TAG);
  ta_log_debug("\n");

  ta_log_debug("attachment_timestamp: %" PRId64 "\n", transaction_attachment_timestamp(tx_obj));
  ta_log_debug("attachment_timestamp_lower: %" PRId64 "\n", transaction_attachment_timestamp_lower(tx_obj));
  ta_log_debug("attachment_timestamp_upper: %" PRId64 "\n", transaction_attachment_timestamp_upper(tx_obj));

  ta_log_debug("nonce: ");
  flex_trit_print(transaction_nonce(tx_obj), NUM_TRITS_NONCE);
  ta_log_debug("\n");

  ta_log_debug("hash: ");
  flex_trit_print(transaction_hash(tx_obj), NUM_TRITS_HASH);
  ta_log_debug("\n");

  ta_log_debug("message: \n");
  flex_trit_print(transaction_message(tx_obj), NUM_TRITS_MESSAGE);
  ta_log_debug("\n");

  memcpy(&tx_obj->loaded_columns_mask, &old_mask, sizeof(field_mask_t));
}

bool transaction_cmp(iota_transaction_t *tx1, iota_transaction_t *tx2) {
  bool equal = true;
  field_mask_t old_mask1 = {}, old_mask2 = {};
  memcpy(&old_mask1, &tx1->loaded_columns_mask, sizeof(field_mask_t));
  memset(&tx1->loaded_columns_mask, 0xFF, sizeof(field_mask_t));
  memcpy(&old_mask2, &tx2->loaded_columns_mask, sizeof(field_mask_t));
  memset(&tx2->loaded_columns_mask, 0xFF, sizeof(field_mask_t));
  uint64_t result = 0;

  // address
  ta_log_debug("address: ");
  result = memcmp(transaction_address(tx1), transaction_address(tx2), NUM_TRYTES_ADDRESS);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("address tx1: ");
    flex_trit_print(transaction_address(tx1), NUM_TRITS_ADDRESS);
    ta_log_debug("\n");
    ta_log_debug("address tx2: ");
    flex_trit_print(transaction_address(tx2), NUM_TRITS_ADDRESS);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("value: ");
  result = transaction_value(tx1) - transaction_value(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("value tx1: %" PRId64 "\n", transaction_value(tx1));
    ta_log_debug("value tx2: %" PRId64 "\n", transaction_value(tx2));
    equal = false;
  }

  ta_log_debug("obsolete tag: ");
  result = memcmp(transaction_obsolete_tag(tx1), transaction_obsolete_tag(tx2), NUM_TRYTES_OBSOLETE_TAG);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("obsolete tag tx1: ");
    flex_trit_print(transaction_obsolete_tag(tx1), NUM_TRITS_OBSOLETE_TAG);
    ta_log_debug("\n");
    ta_log_debug("obsolete tag tx2: ");
    flex_trit_print(transaction_obsolete_tag(tx2), NUM_TRITS_OBSOLETE_TAG);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("timestamp: ");
  result = transaction_timestamp(tx1) - transaction_timestamp(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("timestamp tx1: %" PRId64 "\n", transaction_timestamp(tx1));
    ta_log_debug("timestamp tx2: %" PRId64 "\n", transaction_timestamp(tx2));
    equal = false;
  }

  ta_log_debug("curr_index: ");
  result = transaction_current_index(tx1) - transaction_current_index(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("curr_index tx1: %" PRId64 "\n", transaction_current_index(tx1));
    ta_log_debug("curr_index tx2: %" PRId64 "\n", transaction_current_index(tx2));
    equal = false;
  }

  ta_log_debug("last_index: ");
  result = transaction_last_index(tx1) - transaction_last_index(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("last_index tx1: %" PRId64 "\n", transaction_last_index(tx1));
    ta_log_debug("last_index tx2: %" PRId64 "\n", transaction_last_index(tx2));
    equal = false;
  }

  ta_log_debug("bundle: ");
  result = memcmp(transaction_bundle(tx1), transaction_bundle(tx2), NUM_TRYTES_BUNDLE);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("bundle tx1: ");
    flex_trit_print(transaction_bundle(tx1), NUM_TRITS_BUNDLE);
    ta_log_debug("\n");
    ta_log_debug("bundle tx2: ");
    flex_trit_print(transaction_bundle(tx2), NUM_TRITS_BUNDLE);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("trunk: ");
  result = memcmp(transaction_trunk(tx1), transaction_trunk(tx2), NUM_TRYTES_TRUNK);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("trunk tx1: ");
    flex_trit_print(transaction_trunk(tx1), NUM_TRITS_TRUNK);
    ta_log_debug("\n");
    ta_log_debug("trunk tx2: ");
    flex_trit_print(transaction_trunk(tx2), NUM_TRITS_TRUNK);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("branch: ");
  result = memcmp(transaction_branch(tx1), transaction_branch(tx2), NUM_TRYTES_BRANCH);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("branch tx1: ");
    flex_trit_print(transaction_branch(tx1), NUM_TRITS_BRANCH);
    ta_log_debug("\n");
    ta_log_debug("branch tx2: ");
    flex_trit_print(transaction_branch(tx2), NUM_TRITS_BRANCH);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("tag: ");
  result = memcmp(transaction_tag(tx1), transaction_tag(tx2), NUM_TRYTES_TAG);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("tag tx1: ");
    flex_trit_print(transaction_tag(tx1), NUM_TRITS_TAG);
    ta_log_debug("\n");
    ta_log_debug("tag tx2: ");
    flex_trit_print(transaction_tag(tx2), NUM_TRITS_TAG);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("attachment_timestamp: ");
  result = transaction_attachment_timestamp(tx1) - transaction_attachment_timestamp(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("attachment_timestamp tx1: %" PRId64 "\n", transaction_attachment_timestamp(tx1));
    ta_log_debug("attachment_timestamp tx2: %" PRId64 "\n", transaction_attachment_timestamp(tx2));
    equal = false;
  }

  ta_log_debug("attachment_timestamp_lower: ");
  result = transaction_attachment_timestamp_lower(tx1) - transaction_attachment_timestamp_lower(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("attachment_timestamp_lower tx1: %" PRId64 "\n", transaction_attachment_timestamp_lower(tx1));
    ta_log_debug("attachment_timestamp_lower tx2: %" PRId64 "\n", transaction_attachment_timestamp_lower(tx2));
    equal = false;
  }

  ta_log_debug("attachment_timestamp_upper: ");
  result = transaction_attachment_timestamp_upper(tx1) - transaction_attachment_timestamp_upper(tx2);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("attachment_timestamp_upper tx1: %" PRId64 "\n", transaction_attachment_timestamp_upper(tx1));
    ta_log_debug("attachment_timestamp_upper tx2: %" PRId64 "\n", transaction_attachment_timestamp_upper(tx2));
    equal = false;
  }

  ta_log_debug("nonce: ");
  result = memcmp(transaction_nonce(tx1), transaction_nonce(tx2), NUM_TRYTES_NONCE);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("nonce tx1: ");
    flex_trit_print(transaction_nonce(tx1), NUM_TRITS_NONCE);
    ta_log_debug("\n");
    ta_log_debug("nonce tx2: ");
    flex_trit_print(transaction_nonce(tx2), NUM_TRITS_NONCE);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("hash: ");
  result = memcmp(transaction_hash(tx1), transaction_hash(tx2), NUM_TRYTES_HASH);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("hash tx1: ");
    flex_trit_print(transaction_hash(tx1), NUM_TRITS_HASH);
    ta_log_debug("\n");
    ta_log_debug("hash tx2: ");
    flex_trit_print(transaction_hash(tx2), NUM_TRITS_HASH);
    ta_log_debug("\n");
    equal = false;
  }

  ta_log_debug("message: \n");
  result = memcmp(transaction_message(tx1), transaction_message(tx2), NUM_TRYTES_MESSAGE);
  ta_log_debug("%" PRId64 "\n", result);
  if (result) {
    ta_log_debug("message tx1: ");
    flex_trit_print(transaction_message(tx1), NUM_TRITS_MESSAGE);
    ta_log_debug("\n");
    ta_log_debug("message tx2: ");
    flex_trit_print(transaction_message(tx2), NUM_TRITS_MESSAGE);
    ta_log_debug("\n");
    equal = false;
  }

  memcpy(&tx1->loaded_columns_mask, &old_mask1, sizeof(field_mask_t));
  memcpy(&tx2->loaded_columns_mask, &old_mask2, sizeof(field_mask_t));
  return equal;
}
