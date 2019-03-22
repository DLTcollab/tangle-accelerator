#include "common_core.h"
#include <sys/time.h>
#include "utils/cache.h"
#include "utils/pow.h"

status_t cclient_get_txn_to_approve(const iota_client_service_t* const service,
                                    uint8_t const depth,
                                    ta_get_tips_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  get_transactions_to_approve_req_t* get_txn_req =
      get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res =
      get_transactions_to_approve_res_new();
  if (get_txn_req == NULL || get_txn_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }
  // The depth at which Random Walk starts. Mininal is 3, and max is 15.
  get_transactions_to_approve_req_set_depth(get_txn_req, depth);

  ret = iota_client_get_transactions_to_approve(service, get_txn_req,
                                                get_txn_res);
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

status_t cclient_get_tips(const iota_client_service_t* const service,
                          ta_get_tips_res_t* res) {
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

status_t cclient_prepare_transfer(const iota_client_service_t* const service,
                                  transfer_t** const transfers,
                                  uint32_t const num_transfer,
                                  bundle_transactions_t* out_bundle) {
  if (transfers == NULL || out_bundle == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  iota_transaction_t* TX = transaction_new();
  iota_transaction_t* tx = NULL;
  Kerl kerl = {};
  transfer_ctx_t transfer_ctx = {};
  transfer_iterator_t* transfer_iterator =
      transfer_iterator_new(transfers, num_transfer, &kerl, TX);
  if (transfer_iterator == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  // calculate bundle hash
  transfer_ctx_init(&transfer_ctx, transfers, 1);
  transfer_ctx_hash(&transfer_ctx, &kerl, transfers, 1);

  for (tx = transfer_iterator_next(transfer_iterator); tx;
       tx = transfer_iterator_next(transfer_iterator)) {
    transaction_set_bundle(tx, transfer_ctx.bundle);
    bundle_transactions_add(out_bundle, tx);
  }

done:
  transfer_iterator_free(&transfer_iterator);
  transaction_free(TX);
  return ret;
}

status_t ta_attach_to_tangle(const attach_to_tangle_req_t* const req,
                             attach_to_tangle_res_t* res) {
  status_t ret = SC_OK;
  bundle_transactions_t* bundle = NULL;
  iota_transaction_t tx;
  flex_trit_t* elt = NULL;
  char cache_key[NUM_TRYTES_HASH] = {0};
  char cache_value[NUM_TRYTES_SERIALIZED_TRANSACTION] = {0};
  cache_t* cache = cache_init();
  pow_init();

  // create bundle
  bundle_transactions_new(&bundle);
  HASH_ARRAY_FOREACH(req->trytes, elt) {
    transaction_deserialize_from_trits(&tx, elt, true);
    bundle_transactions_add(bundle, &tx);

    // store transaction to cache
    flex_trits_to_trytes((tryte_t*)cache_key, NUM_TRYTES_HASH,
                         transaction_hash(&tx), NUM_TRITS_HASH, NUM_TRITS_HASH);
    flex_trits_to_trytes(
        (tryte_t*)cache_value, NUM_TRYTES_SERIALIZED_TRANSACTION, elt,
        NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
    ret = cache_set(cache, cache_key, cache_value);
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
  cache_stop(&cache);
  pow_destroy();
  bundle_transactions_free(&bundle);
  return ret;
}

status_t ta_send_trytes(const iota_config_t* const tangle,
                        const iota_client_service_t* const service,
                        hash8019_array_p trytes) {
  status_t ret = SC_OK;
  ta_get_tips_res_t* get_txn_res = ta_get_tips_res_new();
  attach_to_tangle_req_t* attach_req = attach_to_tangle_req_new();
  attach_to_tangle_res_t* attach_res = attach_to_tangle_res_new();
  if (!get_txn_res || !attach_req || !attach_res) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  // get transaction to approve
  ret = cclient_get_txn_to_approve(service, tangle->depth, get_txn_res);
  if (ret) {
    goto done;
  }

  // attach to tangle

  memcpy(attach_req->trunk, hash243_stack_peek(get_txn_res->tips),
         FLEX_TRIT_SIZE_243);
  hash243_stack_pop(&get_txn_res->tips);
  memcpy(attach_req->branch, hash243_stack_peek(get_txn_res->tips),
         FLEX_TRIT_SIZE_243);
  hash243_stack_pop(&get_txn_res->tips);
  attach_req->mwm = tangle->mwm;
  attach_req->trytes = trytes;
  ret = ta_attach_to_tangle(attach_req, attach_res);
  if (ret) {
    goto done;
  }

  // store and broadcast
  ret = iota_client_store_and_broadcast(service,
                                        (store_transactions_req_t*)attach_res);
  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
  }

done:
  ta_get_tips_res_free(&get_txn_res);
  attach_req->trytes = NULL;
  attach_to_tangle_req_free(&attach_req);
  attach_to_tangle_res_free(&attach_res);
  return ret;
}

status_t ta_generate_address(const iota_config_t* const tangle,
                             const iota_client_service_t* const service,
                             ta_generate_address_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(seed_trits, NUM_TRITS_HASH,
                         (const tryte_t*)tangle->seed, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  address_opt_t opt = {.security = 3, .start = 0, .total = 0};

  ret = iota_client_get_new_address(service, seed_trits, opt, &out_address);

  if (ret) {
    ret = SC_CCLIENT_FAILED_RESPONSE;
  } else {
    res->addresses = out_address;
  }
  return ret;
}

status_t ta_send_transfer(const iota_config_t* const tangle,
                          const iota_client_service_t* const service,
                          const ta_send_transfer_req_t* const req,
                          ta_send_transfer_res_t* res) {
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
  transfer_t* transfers[1];
  transfers[0] = transfer_data_new(hash243_queue_peek(req->address),
                                   hash81_queue_peek(req->tag), req->message,
                                   req->msg_len, current_timestamp_ms());
  if (find_tx_req == NULL || find_tx_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  ret = cclient_prepare_transfer(service, transfers, 1, out_bundle);
  if (ret) {
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
  hash_array_free(raw_tx);
  transfer_free(transfers);
  bundle_transactions_free(&out_bundle);
  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

status_t ta_find_transactions_by_tag(const iota_client_service_t* const service,
                                     const char* const req,
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
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)req,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
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

status_t ta_get_transaction_object(const iota_client_service_t* const service,
                                   const char* const req,
                                   ta_get_transaction_object_res_t* res) {
  if (res == NULL) {
    return SC_TA_NULL;
  }

  status_t ret = SC_OK;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trit_t hash_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(hash_trits, NUM_TRITS_HASH, (const tryte_t*)req,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  char cache_value[FLEX_TRIT_SIZE_8019] = {0};

  // get raw transaction data of transaction hashes
  cache_t* cache = cache_init();
  get_trytes_req_t* get_trytes_req = get_trytes_req_new();
  get_trytes_res_t* get_trytes_res = get_trytes_res_new();
  if (cache == NULL || get_trytes_req == NULL || get_trytes_res == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  // get in cache first, then get from full node if no result in cache
  ret = cache_get(cache, req, cache_value);
  if (ret == SC_OK) {
    flex_trits_from_trytes(
        tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
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

    memcpy(tx_trits, hash8019_queue_peek(get_trytes_res->trytes),
           FLEX_TRIT_SIZE_8019);

    // set into cache if get_trytes response is not null trytes
    if (!flex_trits_are_null(tx_trits, FLEX_TRIT_SIZE_8019)) {
      flex_trits_to_trytes(
          (tryte_t*)cache_value, NUM_TRYTES_SERIALIZED_TRANSACTION, tx_trits,
          NUM_TRITS_SERIALIZED_TRANSACTION, NUM_TRITS_SERIALIZED_TRANSACTION);
      cache_set(cache, req, cache_value);
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
  cache_stop(&cache);
  get_trytes_req_free(&get_trytes_req);
  get_trytes_res_free(&get_trytes_res);
  return ret;
}

status_t ta_find_transactions_obj_by_tag(
    const iota_client_service_t* const service, const char* const req,
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
    ta_get_transaction_object_res_t* obj_res =
        ta_get_transaction_object_res_new();
    if (obj_res == NULL) {
      ret = SC_TA_OOM;
      goto done;
    }
    flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, hash_trits,
                         NUM_TRITS_HASH, NUM_TRITS_HASH);
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
             : (transaction_current_index(_lhs) >
                transaction_current_index(_rhs));
}

static void get_first_bundle_from_transactions(
    transaction_array_t const transactions,
    bundle_transactions_t* const bundle) {
  iota_transaction_t* tail = NULL;
  iota_transaction_t* curr_tx = NULL;
  iota_transaction_t* prev = NULL;

  utarray_sort(transactions, idx_sort);
  tail = (iota_transaction_t*)utarray_eltptr(transactions, 0);
  bundle_transactions_add(bundle, tail);

  prev = tail;
  TX_OBJS_FOREACH(transactions, curr_tx) {
    if (transaction_current_index(curr_tx) ==
            (transaction_current_index(prev) + 1) &&
        (memcmp(transaction_hash(curr_tx), transaction_trunk(prev),
                FLEX_TRIT_SIZE_243) == 0)) {
      bundle_transactions_add(bundle, curr_tx);
      prev = curr_tx;
    }
  }
}

status_t ta_get_bundle(const iota_client_service_t* const service,
                       tryte_t const* const bundle_hash,
                       bundle_transactions_t* const bundle) {
  status_t ret = SC_OK;
  iota_transaction_t* curr_tx = NULL;
  flex_trit_t bundle_hash_flex[FLEX_TRIT_SIZE_243];
  transaction_array_t tx_objs = transaction_array_new();
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  if (find_tx_req == NULL) {
    ret = SC_CCLIENT_OOM;
    goto done;
  }

  // find transactions by bundle hash
  flex_trits_from_trytes(bundle_hash_flex, NUM_TRITS_BUNDLE, bundle_hash,
                         NUM_TRITS_HASH, NUM_TRYTES_BUNDLE);
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
