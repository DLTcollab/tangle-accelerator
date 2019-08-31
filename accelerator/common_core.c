/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "common_core.h"
#include <sys/time.h>

#define CC_LOGGER "common_core"

static logger_id_t cc_logger_id;

void cc_logger_init() { cc_logger_id = logger_helper_enable(CC_LOGGER, LOGGER_DEBUG, true); }

int cc_logger_release() {
  logger_helper_release(cc_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(cc_logger_id, "[%s:%d] Destroying logger failed %s.\n", __func__, __LINE__, CC_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t cclient_get_txn_to_approve(const iota_client_service_t* const service, uint8_t const depth,
                                    ta_get_tips_res_t* res) {
  if (res == NULL) {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* get_txn_req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res = get_transactions_to_approve_res_new();
  if (get_txn_req == NULL || get_txn_res == NULL) {
    ret = SC_CCLIENT_OOM;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
    goto done;
  }
  // The depth at which Random Walk starts. Mininal is 3, and max is 15.
  get_transactions_to_approve_req_set_depth(get_txn_req, depth);

  ret = iota_client_get_transactions_to_approve(service, get_txn_req, get_txn_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  ret = hash243_stack_push(&res->tips, get_txn_res->branch);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_HASH");
    goto done;
  }
  ret = hash243_stack_push(&res->tips, get_txn_res->trunk);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_HASH");
  }

done:
  get_transactions_to_approve_res_free(&get_txn_res);
  get_transactions_to_approve_req_free(&get_txn_req);
  return ret;
}

status_t ta_attach_to_tangle(const attach_to_tangle_req_t* const req, attach_to_tangle_res_t* res) {
  status_t ret = SC_OK;
  bundle_transactions_t* bundle = NULL;
  iota_transaction_t tx;
  flex_trit_t* elt = NULL;
  char cache_key[NUM_TRYTES_HASH] = {0};
  char cache_value[NUM_TRYTES_SERIALIZED_TRANSACTION] = {0};

  // create bundle
  bundle_transactions_new(&bundle);
  HASH_ARRAY_FOREACH(req->trytes, elt) {
    transaction_deserialize_from_trits(&tx, elt, true);
    bundle_transactions_add(bundle, &tx);

    // store transaction to cache
    flex_trits_to_trytes((tryte_t*)cache_key, NUM_TRYTES_HASH, transaction_hash(&tx), NUM_TRITS_HASH, NUM_TRITS_HASH);
    flex_trits_to_trytes((tryte_t*)cache_value, NUM_TRYTES_SERIALIZED_TRANSACTION, elt,
                         NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
    ret = cache_set(cache_key, cache_value);
    if (ret != SC_OK && ret != SC_CACHE_OFF) {
      goto done;
    }
  }

  // PoW to bundle
  ret = ta_pow(bundle, req->trunk, req->branch, req->mwm);
  if (ret) {
    goto done;
  }

  // bundle to trytes
  iota_transaction_t* tx_iter = NULL;
  BUNDLE_FOREACH(bundle, tx_iter) {
    flex_trit_t* tx_trytes = transaction_serialize(tx_iter);
    if (tx_trytes) {
      hash_array_push(res->trytes, tx_trytes);
      free(tx_trytes);
      tx_trytes = NULL;
    } else {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
  }

done:
  bundle_transactions_free(&bundle);
  return ret;
}

status_t ta_send_trytes(const iota_config_t* const iconf, const iota_client_service_t* const service,
                        hash8019_array_p trytes) {
  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* tx_approve_req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* tx_approve_res = get_transactions_to_approve_res_new();
  attach_to_tangle_req_t* attach_req = attach_to_tangle_req_new();
  attach_to_tangle_res_t* attach_res = attach_to_tangle_res_new();
  if (!tx_approve_req || !tx_approve_res || !attach_req || !attach_res) {
    ret = SC_CCLIENT_OOM;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
    goto done;
  }

  get_transactions_to_approve_req_set_depth(tx_approve_req, iconf->milestone_depth);
  if (iota_client_get_transactions_to_approve(service, tx_approve_req, tx_approve_res)) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  // copy trytes to attach_req->trytes
  flex_trit_t* elt = NULL;
  HASH_ARRAY_FOREACH(trytes, elt) { attach_to_tangle_req_trytes_add(attach_req, elt); }
  attach_to_tangle_req_init(attach_req, get_transactions_to_approve_res_trunk(tx_approve_res),
                            get_transactions_to_approve_res_branch(tx_approve_res), iconf->mwm);
  if (ta_attach_to_tangle(attach_req, attach_res) != SC_OK) {
    goto done;
  }

  // store and broadcast
  if (iota_client_store_and_broadcast(service, (store_transactions_req_t*)attach_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  // set the value of attach_res->trytes as output trytes result
  memcpy(trytes, attach_res->trytes, hash_array_len(attach_res->trytes) * sizeof(hash8019_array_p));

done:
  get_transactions_to_approve_req_free(&tx_approve_req);
  get_transactions_to_approve_res_free(&tx_approve_res);
  attach_to_tangle_req_free(&attach_req);
  attach_to_tangle_res_free(&attach_res);
  return ret;
}

status_t ta_generate_address(const iota_config_t* const iconf, const iota_client_service_t* const service,
                             ta_generate_address_res_t* res) {
  if (res == NULL) {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(seed_trits, NUM_TRITS_HASH, (const tryte_t*)iconf->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  address_opt_t opt = {.security = 3, .start = 0, .total = 0};

  ret = iota_client_get_new_address(service, seed_trits, opt, &out_address);

  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
  } else {
    res->addresses = out_address;
  }
  return ret;
}

status_t ta_send_transfer(const iota_config_t* const iconf, const iota_client_service_t* const service,
                          const ta_send_transfer_req_t* const req, ta_send_transfer_res_t* res) {
  if (req == NULL || res == NULL) {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  flex_trit_t* serialized_txn;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();
  hash8019_array_p raw_tx = hash8019_array_new();
  iota_transaction_t* txn = NULL;
  bundle_transactions_t* out_bundle = NULL;
  bundle_transactions_new(&out_bundle);
  transfer_array_t* transfers = transfer_array_new();
  if (find_tx_req == NULL || find_tx_res == NULL || transfers == NULL) {
    ret = SC_CCLIENT_OOM;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
    goto done;
  }

  // TODO Maybe we can replace the variable type of message from flex_trit_t to
  // tryte_t
  tryte_t msg_tryte[NUM_TRYTES_SERIALIZED_TRANSACTION];
  flex_trits_to_trytes(msg_tryte, req->msg_len / 3, req->message, req->msg_len, req->msg_len);

  transfer_t transfer = {.value = 0, .timestamp = current_timestamp_ms(), .msg_len = req->msg_len / 3};

  if (transfer_message_set_trytes(&transfer, msg_tryte, transfer.msg_len) != RC_OK) {
    ret = SC_CCLIENT_OOM;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
    goto done;
  }
  memcpy(transfer.address, hash243_queue_peek(req->address), FLEX_TRIT_SIZE_243);
  memcpy(transfer.tag, hash81_queue_peek(req->tag), FLEX_TRIT_SIZE_81);
  transfer_array_add(transfers, &transfer);

  // TODO we may need args `remainder_address`, `inputs`, `timestampe` in the
  // future and declare `security` field in `iota_config_t`
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(seed, NUM_TRITS_HASH, (tryte_t const*)iconf->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  if (iota_client_prepare_transfers(service, seed, 2, transfers, NULL, NULL, false, current_timestamp_ms(),
                                    out_bundle) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  BUNDLE_FOREACH(out_bundle, txn) {
    serialized_txn = transaction_serialize(txn);
    if (serialized_txn == NULL) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
    hash_array_push(raw_tx, serialized_txn);
    free(serialized_txn);
  }

  ret = ta_send_trytes(iconf, service, raw_tx);
  if (ret) {
    goto done;
  }

  txn = (iota_transaction_t*)utarray_front(out_bundle);
  ret = hash243_queue_push(&find_tx_req->bundles, transaction_bundle(txn));
  if (ret) {
    ret = SC_CCLIENT_HASH;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_HASH");
    goto done;
  }

  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  res->hash = find_tx_res->hashes;
  find_tx_res->hashes = NULL;

done:
  transfer_message_free(&transfer);
  hash_array_free(raw_tx);
  transfer_array_free(transfers);
  bundle_transactions_free(&out_bundle);
  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

status_t ta_find_transactions_obj_by_tag(const iota_client_service_t* const service,
                                         const find_transactions_req_t* const req, transaction_array_t* res) {
  if (req == NULL || res == NULL) {
    log_error(cc_logger_id, "[%s:%d]\n", __func__, __LINE__);
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  find_transactions_res_t* txn_res = find_transactions_res_new();
  ta_find_transaction_objects_req_t* obj_req = ta_find_transaction_objects_req_new();
  if (txn_res == NULL || obj_req == NULL) {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    goto done;
  }

  // get transaction hash
  if (iota_client_find_transactions(service, req, txn_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

  hash243_queue_copy(&obj_req->hashes, txn_res->hashes, hash243_queue_count(txn_res->hashes));

  ret = ta_find_transaction_objects(service, obj_req, res);
  if (ret) {
    log_error(cc_logger_id, "[%s:%d]\n", __func__, __LINE__);
    goto done;
  }

done:
  find_transactions_res_free(&txn_res);
  ta_find_transaction_objects_req_free(&obj_req);
  return ret;
}

status_t ta_find_transaction_objects(const iota_client_service_t* const service,
                                     const ta_find_transaction_objects_req_t* const req, transaction_array_t* res) {
  status_t ret = SC_OK;
  flex_trit_t tx_trits[NUM_TRITS_SERIALIZED_TRANSACTION];
  iota_transaction_t* temp = NULL;
  get_trytes_req_t* req_get_trytes = get_trytes_req_new();
  transaction_array_t* uncached_txn_array = transaction_array_new();
  if (req == NULL || res == NULL || req_get_trytes == NULL || uncached_txn_array == NULL) {
    ret = SC_TA_NULL;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    goto done;
  }
  char txn_hash[NUM_TRYTES_HASH + 1] = {0};
  char cache_value[NUM_TRYTES_SERIALIZED_TRANSACTION + 1] = {0};
  txn_hash[NUM_TRYTES_HASH] = '\0';
  cache_value[NUM_TRYTES_SERIALIZED_TRANSACTION] = '\0';

  // append transaction object which is already cached to transaction_array_t
  // if not, append uncached to request object of `iota_client_find_transaction_objectss`
  hash243_queue_entry_t* q_iter = NULL;
  CDL_FOREACH(req->hashes, q_iter) {
    flex_trits_to_trytes((tryte_t*)txn_hash, NUM_TRYTES_HASH, q_iter->hash, NUM_TRITS_HASH, NUM_TRITS_HASH);

    ret = cache_get(txn_hash, cache_value);
    if (ret == SC_OK) {
      flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
                             NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

      // deserialize raw data to transaction object
      temp = transaction_deserialize(tx_trits, true);
      if (temp == NULL) {
        ret = SC_CCLIENT_OOM;
        log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
        goto done;
      }

      transaction_array_push_back(res, temp);
      transaction_free(temp);

      // reset the string `cache_value`
      cache_value[0] = '\0';
    } else {
      if (hash243_queue_push(&req_get_trytes->hashes, q_iter->hash) != RC_OK) {
        ret = SC_CCLIENT_HASH;
        log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_HASH");
        goto done;
      }
      ret = SC_OK;
    }
  }

  if (req_get_trytes->hashes != NULL) {
    if (iota_client_get_transaction_objects(service, req_get_trytes, uncached_txn_array) != RC_OK) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
      goto done;
    }
  }

  // append response of `iota_client_find_transaction_objects` into cache
  flex_trit_t* temp_txn_trits = NULL;
  TX_OBJS_FOREACH(uncached_txn_array, temp) {
    temp_txn_trits = transaction_serialize(temp);
    if (!flex_trits_are_null(temp_txn_trits, FLEX_TRIT_SIZE_8019)) {
      flex_trits_to_trytes((tryte_t*)txn_hash, NUM_TRYTES_HASH, transaction_hash(temp), NUM_TRITS_HASH, NUM_TRITS_HASH);
      flex_trits_to_trytes((tryte_t*)cache_value, NUM_TRYTES_SERIALIZED_TRANSACTION, temp_txn_trits,
                           NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      ret = cache_set(txn_hash, cache_value);
      if (ret != SC_OK) {
        if (ret != SC_CACHE_OFF) {
          goto done;
        }
        ret = SC_OK;
      }

      iota_transaction_t* append_txn = transaction_deserialize(temp_txn_trits, true);
      transaction_array_push_back(res, append_txn);
      transaction_free(append_txn);
    } else {
      ret = SC_CCLIENT_NOT_FOUND;
      log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_NOT_FOUND");
      goto done;
    }
    free(temp_txn_trits);
    temp_txn_trits = NULL;
  }

done:
  get_trytes_req_free(&req_get_trytes);
  transaction_array_free(uncached_txn_array);
  free(temp_txn_trits);
  return ret;
}

static int idx_sort(void const* lhs, void const* rhs) {
  iota_transaction_t* _lhs = (iota_transaction_t*)lhs;
  iota_transaction_t* _rhs = (iota_transaction_t*)rhs;

  return (transaction_current_index(_lhs) < transaction_current_index(_rhs))
             ? -1
             : (transaction_current_index(_lhs) > transaction_current_index(_rhs));
}

static void get_first_bundle_from_transactions(transaction_array_t const* transactions,
                                               bundle_transactions_t* const bundle) {
  iota_transaction_t* tail = NULL;
  iota_transaction_t* curr_tx = NULL;
  iota_transaction_t* prev = NULL;

  utarray_sort(transactions, idx_sort);
  tail = (iota_transaction_t*)utarray_eltptr(transactions, 0);
  bundle_transactions_add(bundle, tail);

  prev = tail;
  TX_OBJS_FOREACH(transactions, curr_tx) {
    if (transaction_current_index(curr_tx) == (transaction_current_index(prev) + 1) &&
        (memcmp(transaction_hash(curr_tx), transaction_trunk(prev), FLEX_TRIT_SIZE_243) == 0)) {
      bundle_transactions_add(bundle, curr_tx);
      prev = curr_tx;
    }
  }
}

status_t ta_get_bundle(const iota_client_service_t* const service, tryte_t const* const bundle_hash,
                       bundle_transactions_t* const bundle) {
  status_t ret = SC_OK;
  flex_trit_t bundle_hash_flex[FLEX_TRIT_SIZE_243];
  transaction_array_t* tx_objs = transaction_array_new();
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  if (find_tx_req == NULL) {
    ret = SC_CCLIENT_OOM;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_OOM");
    goto done;
  }

  // find transactions by bundle hash
  flex_trits_from_trytes(bundle_hash_flex, NUM_TRITS_BUNDLE, bundle_hash, NUM_TRITS_HASH, NUM_TRYTES_BUNDLE);
  hash243_queue_push(&find_tx_req->bundles, bundle_hash_flex);
  ret = iota_client_find_transaction_objects(service, find_tx_req, tx_objs);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    goto done;
  }

  // retreive only first bundle
  get_first_bundle_from_transactions(tx_objs, bundle);

done:
  transaction_array_free(tx_objs);
  find_transactions_req_free(&find_tx_req);
  return ret;
}

status_t ta_send_bundle(const iota_config_t* const iconf, const iota_client_service_t* const service,
                        bundle_transactions_t* const bundle) {
  Kerl kerl;
  kerl_init(&kerl);
  bundle_finalize(bundle, &kerl);
  transaction_array_t* out_tx_objs = transaction_array_new();
  hash8019_array_p raw_trytes = hash8019_array_new();
  iota_transaction_t* curr_tx = NULL;
  flex_trit_t trits_8019[FLEX_TRIT_SIZE_8019];

  BUNDLE_FOREACH(bundle, curr_tx) {
    transaction_serialize_on_flex_trits(curr_tx, trits_8019);
    hash_array_push(raw_trytes, trits_8019);
  }

  ta_send_trytes(iconf, service, raw_trytes);

  hash_array_free(raw_trytes);
  transaction_array_free(out_tx_objs);

  return SC_OK;
}

status_t ta_get_bundle_by_addr(const iota_client_service_t* const service, tryte_t const* const addr,
                               bundle_transactions_t* bundle) {
  status_t ret = SC_OK;
  tryte_t bundle_hash[NUM_TRYTES_BUNDLE];
  find_transactions_req_t* txn_req = find_transactions_req_new();
  find_transactions_res_t* txn_res = find_transactions_res_new();
  ta_find_transaction_objects_req_t* obj_req = ta_find_transaction_objects_req_new();
  transaction_array_t* obj_res = transaction_array_new();

  if (txn_req == NULL || txn_res == NULL || obj_req == NULL || obj_res == NULL) {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_OOM");
    ret = SC_TA_OOM;
    goto done;
  }

  flex_trit_t addr_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(addr_trits, NUM_TRITS_HASH, (const tryte_t*)addr, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  find_transactions_req_address_add(txn_req, addr_trits);

  if (iota_client_find_transactions(service, txn_req, txn_res) != RC_OK) {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CCLIENT_FAILED_RESPONSE");
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  // In case the requested transction hashes is an empty one
  if (hash243_queue_count(txn_res->hashes) > 0) {
    hash243_queue_push(&obj_req->hashes, find_transactions_res_hashes_get(txn_res, 0));
  } else {
    log_error(cc_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MAM_NOT_FOUND");
    ret = SC_MAM_NOT_FOUND;
    goto done;
  }

  ret = ta_find_transaction_objects(service, obj_req, obj_res);
  if (ret != SC_OK) {
    log_error(cc_logger_id, "[%s:%d:%d]\n", __func__, __LINE__, ret);
    goto done;
  }

  iota_transaction_t* curr_tx = transaction_array_at(obj_res, 0);
  flex_trits_to_trytes(bundle_hash, NUM_TRYTES_BUNDLE, transaction_bundle(curr_tx), NUM_TRITS_BUNDLE, NUM_TRITS_BUNDLE);
  ret = ta_get_bundle(service, bundle_hash, bundle);
  if (ret != SC_OK) {
    log_error(cc_logger_id, "[%s:%d:%d]\n", __func__, __LINE__, ret);
    goto done;
  }

done:
  find_transactions_req_free(&txn_req);
  find_transactions_res_free(&txn_res);
  ta_find_transaction_objects_req_free(&obj_req);
  transaction_array_free(obj_res);
  return ret;
}
