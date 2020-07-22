/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "core.h"
#include <sys/time.h>

#define CC_LOGGER "core"

static logger_id_t logger_id;

void cc_logger_init() { logger_id = logger_helper_enable(CC_LOGGER, LOGGER_DEBUG, true); }

int cc_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

status_t ta_attach_to_tangle(const attach_to_tangle_req_t* const req, attach_to_tangle_res_t* res) {
  status_t ret = SC_OK;
  bundle_transactions_t* bundle = NULL;
  iota_transaction_t tx;
  flex_trit_t* elt = NULL;

  // create bundle
  bundle_transactions_new(&bundle);
  HASH_ARRAY_FOREACH(req->trytes, elt) {
    transaction_deserialize_from_trits(&tx, elt, true);
    bundle_transactions_add(bundle, &tx);
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
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

done:
  bundle_transactions_free(&bundle);
  return ret;
}

status_t ta_send_trytes(const ta_config_t* const info, const iota_config_t* const iconf,
                        const iota_client_service_t* const service, hash8019_array_p trytes) {
  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* tx_approve_req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* tx_approve_res = get_transactions_to_approve_res_new();
  attach_to_tangle_req_t* attach_req = attach_to_tangle_req_new();
  attach_to_tangle_res_t* attach_res = attach_to_tangle_res_new();
  if (!tx_approve_req || !tx_approve_res || !attach_req || !attach_res) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  if (is_option_enabled(info, CLI_GTTA)) {
    get_transactions_to_approve_req_set_depth(tx_approve_req, iconf->milestone_depth);
    if (iota_client_get_transactions_to_approve(service, tx_approve_req, tx_approve_res)) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

  // copy trytes to attach_req->trytes
  flex_trit_t* elt = NULL;
  HASH_ARRAY_FOREACH(trytes, elt) { attach_to_tangle_req_trytes_add(attach_req, elt); }
  attach_to_tangle_req_init(attach_req, get_transactions_to_approve_res_trunk(tx_approve_res),
                            get_transactions_to_approve_res_branch(tx_approve_res), iconf->mwm);
  ret = ta_attach_to_tangle(attach_req, attach_res);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // store and broadcast
  if (iota_client_store_and_broadcast(service, (store_transactions_req_t*)attach_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // Clear the old data in the hash_array `trytes`
  utarray_clear(trytes);
  const int attach_res_trytes_len = hash_array_len(attach_res->trytes);
  for (int i = 0; i < attach_res_trytes_len; ++i) {
    // set the value of attach_res->trytes as output trytes result
    hash_array_push(trytes, hash_array_at(attach_res->trytes, i));
  }

done:
  get_transactions_to_approve_req_free(&tx_approve_req);
  get_transactions_to_approve_res_free(&tx_approve_res);
  attach_to_tangle_req_free(&attach_req);
  attach_to_tangle_res_free(&attach_res);
  return ret;
}

status_t ta_send_transfer(const ta_config_t* const info, const iota_config_t* const iconf,
                          const iota_client_service_t* const service, const ta_cache_t* const cache,
                          const ta_send_transfer_req_t* const req, ta_send_transfer_res_t* res) {
  if (req == NULL || res == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
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
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  transfer_t transfer = {.value = 0, .timestamp = current_timestamp_ms(), .msg_len = req->msg_len};

  if (transfer_message_set_trytes(&transfer, req->message, transfer.msg_len) != RC_OK) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  memcpy(transfer.address, hash243_queue_peek(req->address), FLEX_TRIT_SIZE_243);
  memcpy(transfer.tag, hash81_queue_peek(req->tag), FLEX_TRIT_SIZE_81);
  transfer_array_add(transfers, &transfer);

  // TODO we may need args `remainder_address`, `inputs`, `timestamp` in the
  // future and declare `security` field in `iota_config_t`
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(seed, NUM_TRITS_HASH, (tryte_t const*)iconf->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  if (iota_client_prepare_transfers(service, seed, 2, transfers, NULL, NULL, false, current_timestamp_ms(),
                                    out_bundle) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  BUNDLE_FOREACH(out_bundle, txn) {
    serialized_txn = transaction_serialize(txn);
    if (serialized_txn == NULL) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    hash_array_push(raw_tx, serialized_txn);
    free(serialized_txn);
  }

  ret = ta_send_trytes(info, iconf, service, raw_tx);
  if (ret) {
    ta_log_error("Error in ta_send_trytes. Push transaction trytes to buffer.\n");
    res->uuid = (char*)malloc(sizeof(char) * UUID_STR_LEN);
    push_txn_to_buffer(cache, raw_tx, res->uuid);

    txn = (iota_transaction_t*)utarray_front(out_bundle);
    res->address = (tryte_t*)malloc(sizeof(char) * NUM_TRYTES_ADDRESS);
    flex_trits_to_trytes(res->address, NUM_TRYTES_ADDRESS, transaction_address(txn), NUM_TRITS_ADDRESS,
                         NUM_TRITS_ADDRESS);
    goto done;
  }

  txn = (iota_transaction_t*)utarray_front(out_bundle);
  ret = hash243_queue_push(&find_tx_req->bundles, transaction_bundle(txn));
  if (ret) {
    ret = SC_CCLIENT_HASH;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
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
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  status_t ret = SC_OK;
  find_transactions_res_t* txn_res = find_transactions_res_new();
  ta_find_transaction_objects_req_t* obj_req = ta_find_transaction_objects_req_new();
  if (txn_res == NULL || obj_req == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // get transaction hash
  if (iota_client_find_transactions(service, req, txn_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  int txn_res_len = hash243_queue_count(txn_res->hashes);
  for (int i = 0; i < RESULT_SET_LIMIT && i < txn_res_len; i++) {
    hash243_queue_entry_t* tmp_queue_entry = hash243_queue_pop(&txn_res->hashes);
    hash243_queue_push(&obj_req->hashes, tmp_queue_entry->hash);
    free(tmp_queue_entry);
  }

  ret = ta_find_transaction_objects(service, obj_req, res);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

done:
  find_transactions_res_free(&txn_res);
  ta_find_transaction_objects_req_free(&obj_req);
  return ret;
}

status_t ta_get_txn_objects_with_txn_hash(const iota_client_service_t* const service,
                                          find_transactions_req_t* tx_queries, transaction_array_t* tx_objs) {
  status_t ret = SC_OK;
  find_transactions_res_t* find_tx_res = find_transactions_res_new();

  if (iota_client_find_transactions(service, tx_queries, find_tx_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  int limit = hash243_queue_count(find_tx_res->hashes);
  for (int i = 0; i < limit; i++) {
    get_trytes_req_t* tx_obj_queries = get_trytes_req_new();
    hash243_queue_push(&tx_obj_queries->hashes, hash243_queue_at(find_tx_res->hashes, i));
    if (iota_client_get_transaction_objects(service, tx_obj_queries, tx_objs) != RC_OK) {
      get_trytes_req_free(&tx_obj_queries);
      ret = SC_CCLIENT_FAILED_RESPONSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    get_trytes_req_free(&tx_obj_queries);
  }

done:
  find_transactions_res_free(&find_tx_res);
  return ret;
}

status_t ta_find_transaction_objects(const iota_client_service_t* const service,
                                     const ta_find_transaction_objects_req_t* const req, transaction_array_t* res) {
  status_t ret = SC_OK;
  flex_trit_t tx_trits[NUM_TRITS_SERIALIZED_TRANSACTION];
  iota_transaction_t* temp = NULL;
  get_trytes_req_t* req_get_trytes = get_trytes_req_new();
  transaction_array_t* uncached_txn_array = transaction_array_new();
  flex_trit_t* temp_txn_trits = NULL;
  if (req == NULL || res == NULL || req_get_trytes == NULL || uncached_txn_array == NULL) {
    ret = SC_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  char txn_hash[NUM_TRYTES_HASH + 1] = {0};
  char* cache_value = NULL;
  txn_hash[NUM_TRYTES_HASH] = '\0';

  // append transaction object which is already cached to transaction_array_t
  // if not, append uncached to request object of `iota_client_find_transaction_objects`
  hash243_queue_entry_t* q_iter = NULL;
  CDL_FOREACH(req->hashes, q_iter) {
    flex_trits_to_trytes((tryte_t*)txn_hash, NUM_TRYTES_HASH, q_iter->hash, NUM_TRITS_HASH, NUM_TRITS_HASH);

    ret = cache_get(txn_hash, &cache_value);
    if (ret == SC_OK) {
      flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
                             NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

      // deserialize raw data to transaction object
      temp = transaction_deserialize(tx_trits, true);
      if (temp == NULL) {
        ret = SC_OOM;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }

      transaction_array_push_back(res, temp);
      transaction_free(temp);

      free(cache_value);
      cache_value = NULL;
    } else {
      if (hash243_queue_push(&req_get_trytes->hashes, q_iter->hash) != RC_OK) {
        ret = SC_CCLIENT_HASH;
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      ret = SC_OK;
    }
  }

  if (req_get_trytes->hashes != NULL) {
    int limit = hash243_queue_count(req_get_trytes->hashes);
    for (int i = 0; i < limit; i++) {
      get_trytes_req_t* tmp_trytes_req = get_trytes_req_new();
      transaction_array_t* tmp_txn_array = transaction_array_new();

      hash243_queue_push(&tmp_trytes_req->hashes, hash243_queue_at(req_get_trytes->hashes, i));
      if (iota_client_get_transaction_objects(service, tmp_trytes_req, uncached_txn_array) != RC_OK) {
        ret = SC_CCLIENT_FAILED_RESPONSE;
        ta_log_error("%s\n", ta_error_to_string(ret));
        get_trytes_req_free(&tmp_trytes_req);
        transaction_array_free(tmp_txn_array);
        goto done;
      }
      transaction_array_push_back(uncached_txn_array, transaction_array_at(uncached_txn_array, 0));
      transaction_array_free(tmp_txn_array);
      get_trytes_req_free(&tmp_trytes_req);
    }
  }

  // append response of `iota_client_find_transaction_objects` into cache
  TX_OBJS_FOREACH(uncached_txn_array, temp) {
    temp_txn_trits = transaction_serialize(temp);
    if (!flex_trits_are_null(temp_txn_trits, FLEX_TRIT_SIZE_8019)) {
      // TODO Save fetched transaction object in a set.

      iota_transaction_t* append_txn = transaction_deserialize(temp_txn_trits, true);
      transaction_array_push_back(res, append_txn);
      transaction_free(append_txn);
    } else {
      ret = SC_CCLIENT_NOT_FOUND;
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    free(temp_txn_trits);
    temp_txn_trits = NULL;
  }

done:
  get_trytes_req_free(&req_get_trytes);
  transaction_array_free(uncached_txn_array);
  free(temp_txn_trits);
  free(cache_value);
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
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // find transactions by bundle hash
  flex_trits_from_trytes(bundle_hash_flex, NUM_TRITS_BUNDLE, bundle_hash, NUM_TRITS_HASH, NUM_TRYTES_BUNDLE);
  hash243_queue_push(&find_tx_req->bundles, bundle_hash_flex);
  ret = ta_get_txn_objects_with_txn_hash(service, find_tx_req, tx_objs);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // retreive only first bundle
  get_first_bundle_from_transactions(tx_objs, bundle);

done:
  transaction_array_free(tx_objs);
  find_transactions_req_free(&find_tx_req);
  return ret;
}

status_t ta_send_bundle(const ta_config_t* const info, const iota_config_t* const iconf,
                        const iota_client_service_t* const service, bundle_transactions_t* const bundle) {
  status_t ret = SC_OK;
  Kerl kerl;
  kerl_init(&kerl);
  bundle_finalize(bundle, &kerl);
  transaction_array_t* out_tx_objs = transaction_array_new();
  hash8019_array_p raw_trytes = hash8019_array_new();
  flex_trit_t trits_8019[FLEX_TRIT_SIZE_8019];

  iota_transaction_t* curr_tx = NULL;
  BUNDLE_FOREACH(bundle, curr_tx) {
    transaction_serialize_on_flex_trits(curr_tx, trits_8019);
    hash_array_push(raw_trytes, trits_8019);
  }

  ret = ta_send_trytes(info, iconf, service, raw_trytes);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  hash_array_free(raw_trytes);
  transaction_array_free(out_tx_objs);

  return SC_OK;
}

status_t ta_get_bundles_by_addr(const iota_client_service_t* const service, tryte_t const* const addr,
                                bundle_array_t* bundle_array) {
  status_t ret = SC_OK;
  tryte_t bundle_hash[NUM_TRYTES_BUNDLE + 1] = {0};
  find_transactions_req_t* txn_req = find_transactions_req_new();
  find_transactions_res_t* txn_res = find_transactions_res_new();
  ta_find_transaction_objects_req_t* obj_req = ta_find_transaction_objects_req_new();
  transaction_array_t* obj_res = transaction_array_new();

  if (txn_req == NULL || txn_res == NULL || obj_req == NULL || obj_res == NULL) {
    ret = SC_OOM;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  flex_trit_t addr_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(addr_trits, NUM_TRITS_HASH, (const tryte_t*)addr, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  find_transactions_req_address_add(txn_req, addr_trits);

  if (iota_client_find_transactions(service, txn_req, txn_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // In case the requested transaction hashes is an empty one
  if (hash243_queue_count(txn_res->hashes) > 0) {
    hash243_queue_copy(&obj_req->hashes, txn_res->hashes, hash243_queue_count(txn_res->hashes));
  } else {
    ret = SC_CCLIENT_NOT_FOUND;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  ret = ta_find_transaction_objects(service, obj_req, obj_res);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  iota_transaction_t* curr_tx = NULL;
  bundle_transactions_t* bundle = NULL;
  hash243_set_t bundle_hash_set = NULL;
  TX_OBJS_FOREACH(obj_res, curr_tx) {
    if (!hash243_set_contains(bundle_hash_set, transaction_bundle(curr_tx))) {
      bundle_transactions_new(&bundle);
      flex_trits_to_trytes(bundle_hash, NUM_TRYTES_BUNDLE, transaction_bundle(curr_tx), NUM_TRITS_BUNDLE,
                           NUM_TRITS_BUNDLE);
      ret = ta_get_bundle(service, bundle_hash, bundle);
      if (ret != SC_OK) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        bundle_transactions_free(&bundle);
        goto done;
      }

      bundle_array_add(bundle_array, bundle);
      hash243_set_add(&bundle_hash_set, transaction_bundle(curr_tx));
      bundle_transactions_free(&bundle);
    }
  }
  hash243_set_free(&bundle_hash_set);

done:
  find_transactions_req_free(&txn_req);
  find_transactions_res_free(&txn_res);
  ta_find_transaction_objects_req_free(&obj_req);
  transaction_array_free(obj_res);
  return ret;
}

status_t ta_get_node_status(const iota_client_service_t* const service) {
  status_t ret = SC_OK;
  char const* const getnodeinfo = "{\"command\": \"getNodeInfo\"}";
  char* node_result = NULL;
  char_buffer_t* res_buff = char_buffer_new();
  char_buffer_t* req_buff = char_buffer_new();
  char_buffer_set(req_buff, getnodeinfo);

  // Check IOTA full node connection
  retcode_t result = iota_service_query(service, req_buff, res_buff);
  if (result != RC_OK) {
    ta_log_error("%s\n", error_2_string(result));
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }
  str_from_char_buffer(res_buff, &node_result);

  // Check whether IOTA full node is at the latest milestone
  int latestMilestoneIndex, latestSolidSubtangleMilestoneIndex;
  ret = get_node_status_milestone_deserialize(node_result, &latestMilestoneIndex, &latestSolidSubtangleMilestoneIndex);
  if (ret != SC_OK) {
    ta_log_error("check iri connection failed deserialized. status code: %d\n", ret);
    goto done;
  }

  // The tolerant difference between latestSolidSubtangleMilestoneIndex and latestMilestoneIndex is less equal than 1.
  if ((latestSolidSubtangleMilestoneIndex - latestMilestoneIndex) > 1) {
    ret = SC_CORE_NODE_UNSYNC;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  free(node_result);
  char_buffer_free(req_buff);
  char_buffer_free(res_buff);

  return ret;
}

status_t push_txn_to_buffer(const ta_cache_t* const cache, hash8019_array_p raw_txn_flex_trit_array, char* uuid) {
  status_t ret = SC_OK;
  if (!uuid) {
    ret = SC_NULL;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  uuid_t bin_uuid;
  uuid_generate_random(bin_uuid);
  uuid_unparse(bin_uuid, uuid);
  if (!uuid[0]) {
    ta_log_error("%s\n", "Failed to generate UUID");
    goto done;
  }

  const int len = hash_array_len(raw_txn_flex_trit_array);
  // We assume all the transactions in a single hash_array would be in the same bundle, since we buffer transaction only
  // when 'ta_send_trytes()' fails, it implies 'ta_send_trytes()' can send only one bundle
  // each time.
  for (int i = 0; i < len; ++i) {
    ret = cache_list_push(uuid, UUID_STR_LEN - 1, hash_array_at(raw_txn_flex_trit_array, i),
                          NUM_FLEX_TRITS_SERIALIZED_TRANSACTION);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  }

  ret = cache_list_push(cache->buffer_list_name, strlen(cache->buffer_list_name), uuid, UUID_STR_LEN - 1);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  return ret;
}

status_t ta_fetch_txn_with_uuid(const ta_cache_t* const cache, const char* const uuid,
                                ta_fetch_buffered_request_status_res_t* res) {
  status_t ret = SC_OK;
  if (pthread_rwlock_tryrdlock(cache->rwlock)) {
    ret = SC_CACHE_LOCK_FAILURE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  bool exist = false;
  ret = cache_list_exist(cache->buffer_list_name, uuid, UUID_STR_LEN - 1, &exist);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
    goto done;
  }
  if (exist) {
    res->status = UNSENT;
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
    goto done;
  }

  ret = cache_list_exist(cache->complete_list_name, uuid, UUID_STR_LEN - 1, &exist);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }
  if (exist) {
    res->status = SENT;

    int len = 0;
    ret = cache_list_size(uuid, &len);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }

    for (int i = 0; i < len; ++i) {
      flex_trit_t txn_flex_trits[NUM_FLEX_TRITS_SERIALIZED_TRANSACTION + 1];
      ret = cache_list_at(uuid, i, NUM_FLEX_TRITS_SERIALIZED_TRANSACTION, (char*)txn_flex_trits);
      if (ret) {
        ta_log_error("%s\n", ta_error_to_string(ret));
        goto done;
      }
      iota_transaction_t* txn = transaction_deserialize(txn_flex_trits, false);

      if (bundle_transactions_add(res->bundle, txn) != RC_OK) {
        ret = SC_NULL;
        ta_log_error("%s\n", "Failed to add transaction to bundle.");
        free(txn);
        goto done;
      }
      free(txn);
    }

    ret = cache_del(uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    char pop_uuid[UUID_STR_LEN];
    ret = cache_list_pop(cache->complete_list_name, pop_uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  } else {
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
  }

done:
  return ret;
}

status_t ta_fetch_mam_with_uuid(const ta_cache_t* const cache, const char* const uuid,
                                ta_fetch_buffered_request_status_res_t* res) {
  status_t ret = SC_OK;
  if (pthread_rwlock_tryrdlock(cache->rwlock)) {
    ret = SC_CACHE_LOCK_FAILURE;
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  bool exist = false;
  ret = cache_list_exist(cache->mam_buffer_list_name, uuid, UUID_STR_LEN - 1, &exist);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
    goto done;
  }
  if (exist) {
    res->status = UNSENT;
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
    goto done;
  }

  ret = cache_list_exist(cache->mam_complete_list_name, uuid, UUID_STR_LEN - 1, &exist);
  if (ret) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
    goto done;
  }
  if (exist) {
    res->status = SENT;

    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }

    ret = cache_get(uuid, &res->mam_result);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    ret = cache_del(uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }

    char pop_uuid[UUID_STR_LEN];
    ret = cache_list_pop(cache->mam_complete_list_name, pop_uuid);
    if (ret) {
      ta_log_error("%s\n", ta_error_to_string(ret));
      goto done;
    }
  } else {
    if (pthread_rwlock_unlock(cache->rwlock)) {
      ta_log_error("%s\n", ta_error_to_string(SC_CACHE_LOCK_FAILURE));
    }
  }

done:
  return ret;
}
