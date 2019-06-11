#include "common_core.h"
#include <sys/time.h>

status_t cclient_get_txn_to_approve(const iota_client_service_t* const service, uint8_t const depth,
                                    ta_get_tips_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* get_txn_req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res = get_transactions_to_approve_res_new();
  if (get_txn_req == NULL || get_txn_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }
  // The depth at which Random Walk starts. Mininal is 3, and max is 15.
  get_transactions_to_approve_req_set_depth(get_txn_req, depth);

  ret = iota_client_get_transactions_to_approve(service, get_txn_req, get_txn_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  ret = hash243_stack_push(&res->tips, get_txn_res->branch);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    goto done;
  }
  ret = hash243_stack_push(&res->tips, get_txn_res->trunk);
  if (ret) {
    ret = SC_CCLIENT_HASH;
  }

done:
  get_transactions_to_approve_res_free(&get_txn_res);
  get_transactions_to_approve_req_free(&get_txn_req);
  return ret;
}

status_t cclient_get_tips(const iota_client_service_t* const service, ta_get_tips_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }
  status_t ret = SC_OK;
  get_tips_res_t* get_tips_res = get_tips_res_new();
  if (get_tips_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  ret = iota_client_get_tips(service, get_tips_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }
  res->tips = get_tips_res->hashes;
  get_tips_res->hashes = NULL;

done:
  get_tips_res_free(&get_tips_res);
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
    if (ret) {
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
      goto done;
    }
  }

done:
  bundle_transactions_free(&bundle);
  return ret;
}

status_t ta_send_trytes(const iota_config_t* const tangle, const iota_client_service_t* const service,
                        hash8019_array_p trytes) {
  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* tx_approve_req = get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* tx_approve_res = get_transactions_to_approve_res_new();
  attach_to_tangle_req_t* attach_req = attach_to_tangle_req_new();
  attach_to_tangle_res_t* attach_res = attach_to_tangle_res_new();
  if (!tx_approve_req || !tx_approve_res || !attach_req || !attach_res) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  get_transactions_to_approve_req_set_depth(tx_approve_req, tangle->milestone_depth);
  if (iota_client_get_transactions_to_approve(service, tx_approve_req, tx_approve_res)) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  // copy trytes to attach_req->trytes
  flex_trit_t* elt = NULL;
  HASH_ARRAY_FOREACH(trytes, elt) { attach_to_tangle_req_trytes_add(attach_req, elt); }
  attach_to_tangle_req_init(attach_req, get_transactions_to_approve_res_trunk(tx_approve_res),
                            get_transactions_to_approve_res_branch(tx_approve_res), tangle->mwm);
  if (ta_attach_to_tangle(attach_req, attach_res) != SC_OK) {
    goto done;
  }

  // store and broadcast
  if (iota_client_store_and_broadcast(service, (store_transactions_req_t*)attach_res) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
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

status_t ta_generate_address(const iota_config_t* const tangle, const iota_client_service_t* const service,
                             ta_generate_address_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(seed_trits, NUM_TRITS_HASH, (const tryte_t*)tangle->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  address_opt_t opt = {.security = 3, .start = 0, .total = 0};

  ret = iota_client_get_new_address(service, seed_trits, opt, &out_address);

  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
  } else {
    res->addresses = out_address;
  }
  return ret;
}

status_t ta_send_transfer(const iota_config_t* const tangle, const iota_client_service_t* const service,
                          const ta_send_transfer_req_t* const req, ta_send_transfer_res_t* res) {
  if (req == NULL || res == NULL) {
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
    goto done;
  }

  // TODO Maybe we can replace the variable type of message from flex_trit_t to
  // tryte_t
  tryte_t msg_tryte[NUM_TRYTES_SERIALIZED_TRANSACTION];
  flex_trits_to_trytes(msg_tryte, req->msg_len / 3, req->message, req->msg_len, req->msg_len);

  transfer_t transfer = {.value = 0, .timestamp = current_timestamp_ms(), .msg_len = req->msg_len / 3};

  if (transfer_message_set_trytes(&transfer, msg_tryte, transfer.msg_len) != RC_OK) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }
  memcpy(transfer.address, hash243_queue_peek(req->address), FLEX_TRIT_SIZE_243);
  memcpy(transfer.tag, hash81_queue_peek(req->tag), FLEX_TRIT_SIZE_81);
  transfer_array_add(transfers, &transfer);

  // TODO we may need args `remainder_address`, `inputs`, `timestampe` in the
  // future and declare `security` field in `iota_config_t`
  flex_trit_t seed[NUM_FLEX_TRITS_ADDRESS];
  flex_trits_from_trytes(seed, NUM_TRITS_HASH, (tryte_t const*)tangle->seed, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  if (iota_client_prepare_transfers(service, seed, 2, transfers, NULL, NULL, false, current_timestamp_ms(),
                                    out_bundle) != RC_OK) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  BUNDLE_FOREACH(out_bundle, txn) {
    serialized_txn = transaction_serialize(txn);
    if (serialized_txn == NULL) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      goto done;
    }
    utarray_insert(raw_tx, serialized_txn, 0);
    free(serialized_txn);
  }

  ret = ta_send_trytes(tangle, service, raw_tx);
  if (ret) {
    goto done;
  }

  txn = (iota_transaction_t*)utarray_front(out_bundle);
  ret = hash243_queue_push(&find_tx_req->bundles, transaction_bundle(txn));
  if (ret) {
    ret = SC_CCLIENT_HASH;
    goto done;
  }

  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
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

status_t ta_find_transactions_by_tag(const iota_client_service_t* const service, const char* const req,
                                     ta_find_transactions_res_t* res) {
  if (req == NULL || res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();
  if (find_tx_req == NULL || find_tx_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  flex_trit_t tag_trits[NUM_TRITS_TAG];
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)req, NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  ret = hash81_queue_push(&find_tx_req->tags, tag_trits);
  if (ret) {
    ret = SC_CCLIENT_HASH;
    goto done;
  }
  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  res->hashes = find_tx_res->hashes;
  find_tx_res->hashes = NULL;

done:
  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

status_t ta_get_transaction_object(const iota_client_service_t* const service, const char* const req,
                                   ta_get_transaction_object_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trit_t hash_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(hash_trits, NUM_TRITS_HASH, (const tryte_t*)req, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  char cache_value[FLEX_TRIT_SIZE_8019] = {0};

  // get raw transaction data of transaction hashes
  get_trytes_req_t* get_trytes_req = get_trytes_req_new();
  get_trytes_res_t* get_trytes_res = get_trytes_res_new();
  if (get_trytes_req == NULL || get_trytes_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  // get in cache first, then get from full node if no result in cache
  ret = cache_get(req, cache_value);
  if (ret == SC_OK) {
    flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
                           NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  } else {
    ret = hash243_queue_push(&get_trytes_req->hashes, hash_trits);
    if (ret) {
      ret = SC_CCLIENT_HASH;
      goto done;
    }

    ret = iota_client_get_trytes(service, get_trytes_req, get_trytes_res);
    if (ret) {
      ret = SC_CCLIENT_FAILED_RESPONSE;
      goto done;
    }

    memcpy(tx_trits, hash8019_queue_peek(get_trytes_res->trytes), FLEX_TRIT_SIZE_8019);

    // set into cache if get_trytes response is not null trytes
    if (!flex_trits_are_null(tx_trits, FLEX_TRIT_SIZE_8019)) {
      flex_trits_to_trytes((tryte_t*)cache_value, NUM_TRYTES_SERIALIZED_TRANSACTION, tx_trits,
                           NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      cache_set(req, cache_value);
    } else {
      ret = SC_CCLIENT_NOT_FOUND;
      goto done;
    }
  }

  // deserialize raw data to transaction object
  res->txn = transaction_deserialize(tx_trits, 0);
  if (res->txn == NULL) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }
  transaction_set_hash(res->txn, hash_trits);

done:
  get_trytes_req_free(&get_trytes_req);
  get_trytes_res_free(&get_trytes_res);
  return ret;
}

status_t ta_find_transactions_obj_by_tag(const iota_client_service_t* const service, const char* const req,
                                         ta_find_transactions_obj_res_t* res) {
  if (req == NULL || res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  char hash_trytes[NUM_TRYTES_HASH + 1];
  flex_trit_t* hash_trits = NULL;

  ta_find_transactions_res_t* hash_res = ta_find_transactions_res_new();
  if (hash_res == NULL) {
    ret = SC_TA_OOM;
    goto done;
  }

  // get transaction hash
  ret = ta_find_transactions_by_tag(service, req, hash_res);
  if (ret) {
    goto done;
  }

  // get transaction obj
  for (hash_trits = hash243_queue_peek(hash_res->hashes); hash_trits != NULL;
       hash_trits = hash243_queue_peek(hash_res->hashes)) {
    ta_get_transaction_object_res_t* obj_res = ta_get_transaction_object_res_new();
    if (obj_res == NULL) {
      ret = SC_TA_OOM;
      goto done;
    }
    flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, hash_trits, NUM_TRITS_HASH, NUM_TRITS_HASH);
    hash243_queue_pop(&hash_res->hashes);
    hash_trytes[NUM_TRYTES_HASH] = '\0';
    ret = ta_get_transaction_object(service, hash_trytes, obj_res);
    if (ret) {
      break;
    }
    utarray_push_back(res->txn_obj, obj_res->txn);
    ta_get_transaction_object_res_free(&obj_res);
  }

done:
  ta_find_transactions_res_free(&hash_res);
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
    goto done;
  }

  // find transactions by bundle hash
  flex_trits_from_trytes(bundle_hash_flex, NUM_TRITS_BUNDLE, bundle_hash, NUM_TRITS_HASH, NUM_TRYTES_BUNDLE);
  hash243_queue_push(&find_tx_req->bundles, bundle_hash_flex);
  ret = iota_client_find_transaction_objects(service, find_tx_req, tx_objs);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
    goto done;
  }

  // retreive only first bundle
  get_first_bundle_from_transactions(tx_objs, bundle);

done:
  transaction_array_free(tx_objs);
  find_transactions_req_free(&find_tx_req);
  return ret;
}

status_t ta_send_bundle(const iota_config_t* const tangle, const iota_client_service_t* const service,
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

  ta_send_trytes(tangle, service, raw_trytes);

  hash_array_free(raw_trytes);
  transaction_array_free(out_tx_objs);

  return SC_OK;
}
