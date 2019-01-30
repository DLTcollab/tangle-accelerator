#include "common_core.h"
#include <sys/time.h>
#include "cache/cache.h"
#include "utils/pow.h"

int cclient_get_txn_to_approve(const iota_client_service_t* const service,
                               ta_get_tips_res_t* res) {
  if (res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  get_transactions_to_approve_req_t* get_txn_req =
      get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res =
      get_transactions_to_approve_res_new();
  if (get_txn_req == NULL || get_txn_res == NULL) {
    ret = -1;
    goto done;
  }
  // The depth at which Random Walk starts. Mininal is 3, and max is 15.
  get_transactions_to_approve_req_set_depth(get_txn_req, 3);

  ret = iota_client_get_transactions_to_approve(service, get_txn_req,
                                                get_txn_res);
  if (ret) {
    goto done;
  }

  ret = hash243_stack_push(&res->tips, get_txn_res->branch);
  if (ret) {
    goto done;
  }
  ret = hash243_stack_push(&res->tips, get_txn_res->trunk);

done:
  get_transactions_to_approve_res_free(&get_txn_res);
  get_transactions_to_approve_req_free(&get_txn_req);
  return ret;
}

int cclient_get_tips(const iota_client_service_t* const service,
                     ta_get_tips_res_t* res) {
  if (res == NULL) {
    return -1;
  }
  retcode_t ret = RC_OK;
  get_tips_res_t* get_tips_res = get_tips_res_new();
  if (get_tips_res == NULL) {
    ret = -1;
    goto done;
  }

  ret = iota_client_get_tips(service, get_tips_res);
  if (ret) {
    goto done;
  }
  res->tips = get_tips_res->hashes;
  get_tips_res->hashes = NULL;

done:
  get_tips_res_free(&get_tips_res);
  return ret;
}

int cclient_prepare_transfer(const iota_client_service_t* const service,
                             transfer_t** const transfers,
                             uint32_t const num_transfer,
                             bundle_transactions_t* out_bundle) {
  if (transfers == NULL || out_bundle == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  iota_transaction_t* TX = transaction_new();
  iota_transaction_t* tx = NULL;
  Kerl kerl = {};
  transfer_iterator_t* transfer_iterator =
      transfer_iterator_new(transfers, num_transfer, &kerl, TX);
  if (transfer_iterator == NULL) {
    ret = -1;
    goto done;
  }

  for (tx = transfer_iterator_next(transfer_iterator); tx;
       tx = transfer_iterator_next(transfer_iterator)) {
    tx->loaded_columns_mask |= MASK_ALL_COLUMNS;
    bundle_transactions_add(out_bundle, tx);
  }

  bundle_finalize(out_bundle, &kerl);

done:
  transfer_iterator_free(&transfer_iterator);
  transaction_free(TX);
  return ret;
}

int ta_attach_to_tangle(const attach_to_tangle_req_t* const req,
                        attach_to_tangle_res_t* res) {
  retcode_t ret = RC_OK;
  bundle_transactions_t* bundle = NULL;
  iota_transaction_t tx;
  flex_trit_t* elt = NULL;

  // create bundle
  bundle_transactions_new(&bundle);
  HASH_ARRAY_FOREACH(req->trytes, elt) {
    transaction_deserialize_from_trits(&tx, elt, false);
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
      ret = RC_CCLIENT_TX_DESERIALIZE_FAILED;
      goto done;
    }
  }

done:
  bundle_transactions_free(&bundle);
  return ret;
}

int ta_send_trytes(const iota_client_service_t* const service,
                   hash8019_array_p trytes) {
  retcode_t ret = RC_OK;
  get_transactions_to_approve_req_t* get_txn_req =
      get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res =
      get_transactions_to_approve_res_new();
  attach_to_tangle_req_t* attach_req = attach_to_tangle_req_new();
  attach_to_tangle_res_t* attach_res = attach_to_tangle_res_new();
  if (!get_txn_req || !get_txn_res || !attach_req || !attach_res) {
    ret = -1;
    goto done;
  }

  // get transaction to approve
  get_transactions_to_approve_req_set_depth(get_txn_req, DEPTH);
  ret = iota_client_get_transactions_to_approve(service, get_txn_req,
                                                get_txn_res);
  if (ret) {
    goto done;
  }

  // attach to tangle
  memcpy(attach_req->trunk, get_txn_res->trunk, FLEX_TRIT_SIZE_243);
  memcpy(attach_req->branch, get_txn_res->branch, FLEX_TRIT_SIZE_243);
  attach_req->mwm = MWM;
  attach_req->trytes = trytes;
  // ret = ta_attach_to_tangle(attach_req, attach_res);
  if (ret) {
    goto done;
  }

  // store and broadcast
  ret = iota_client_store_and_broadcast(service,
                                        (store_transactions_req_t*)attach_res);

done:
  get_transactions_to_approve_req_free(&get_txn_req);
  get_transactions_to_approve_res_free(&get_txn_res);
  attach_req->trytes = NULL;
  attach_to_tangle_req_free(&attach_req);
  attach_to_tangle_res_free(&attach_res);
  return ret;
}

int ta_generate_address(const iota_client_service_t* const service,
                        ta_generate_address_res_t* res) {
  if (res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(seed_trits, NUM_TRITS_HASH, (const tryte_t*)SEED,
                         NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  address_opt_t opt = {.security = 3, .start = 0, .total = 0};

  ret = iota_client_get_new_address(service, seed_trits, opt, &out_address);

  if (ret == RC_OK) {
    res->addresses = out_address;
  }
  return ret;
}

int ta_send_transfer(const iota_client_service_t* const service,
                     const ta_send_transfer_req_t* const req,
                     ta_send_transfer_res_t* res) {
  if (req == NULL || res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
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
    ret = -1;
    goto done;
  }

  ret = cclient_prepare_transfer(service, transfers, 1, out_bundle);
  if (ret) {
    goto done;
  }

  BUNDLE_FOREACH(out_bundle, txn) {
    serialized_txn = transaction_serialize(txn);
    if (serialized_txn == NULL) {
      ret = -1;
      goto done;
    }
    utarray_insert(raw_tx, serialized_txn, 0);
    free(serialized_txn);
  }

  ret = ta_send_trytes(service, raw_tx);
  if (ret) {
    goto done;
  }

  txn = (iota_transaction_t*)utarray_front(out_bundle);
  ret = hash243_queue_push(&find_tx_req->bundles, transaction_bundle(txn));
  if (ret) {
    goto done;
  }

  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
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

int ta_find_transactions_by_tag(const iota_client_service_t* const service,
                                const char* const req,
                                ta_find_transactions_res_t* res) {
  if (req == NULL || res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();
  if (find_tx_req == NULL || find_tx_res == NULL) {
    ret = -1;
    goto done;
  }

  flex_trit_t tag_trits[NUM_TRITS_TAG];
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)req,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  ret = hash81_queue_push(&find_tx_req->tags, tag_trits);
  if (ret) {
    goto done;
  }
  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);
  if (ret) {
    goto done;
  }

  res->hashes = find_tx_res->hashes;
  find_tx_res->hashes = NULL;

done:
  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

int ta_get_transaction_object(const iota_client_service_t* const service,
                              const char* const req,
                              ta_get_transaction_object_res_t* res) {
  if (res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trit_t hash_trits[NUM_TRITS_HASH];
  char cache_value[FLEX_TRIT_SIZE_8019] = {0};

  // get raw transaction data of transaction hashes
  cache_t* cache = cache_init();
  get_trytes_req_t* get_trytes_req = get_trytes_req_new();
  get_trytes_res_t* get_trytes_res = get_trytes_res_new();
  if (cache == NULL || get_trytes_req == NULL || get_trytes_res == NULL) {
    goto done;
  }

  // get in cache first, then get from full node if no result in cache
  ret = cache_get(cache, req, cache_value);
  if (ret == RC_OK) {
    flex_trits_from_trytes(
        tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
        NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  } else {
    flex_trits_from_trytes(hash_trits, NUM_TRITS_HASH, (const tryte_t*)req,
                           NUM_TRYTES_HASH, NUM_TRYTES_HASH);
    ret = hash243_queue_push(&get_trytes_req->hashes, hash_trits);
    if (ret) {
      goto done;
    }

    ret = iota_client_get_trytes(service, get_trytes_req, get_trytes_res);
    if (ret) {
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
    }
  }

  // deserialize raw data to transaction object
  res->txn = transaction_deserialize(tx_trits, 0);
  if (res->txn == NULL) {
    ret = -1;
    goto done;
  }
  transaction_set_hash(res->txn, hash_trits);

done:
  cache_stop(&cache);
  get_trytes_req_free(&get_trytes_req);
  get_trytes_res_free(&get_trytes_res);
  return ret;
}

int ta_find_transactions_obj_by_tag(const iota_client_service_t* const service,
                                    const char* const req,
                                    ta_find_transactions_obj_res_t* res) {
  if (req == NULL || res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  char hash_trytes[NUM_TRYTES_HASH + 1];
  flex_trit_t* hash_trits = NULL;

  ta_find_transactions_res_t* hash_res = ta_find_transactions_res_new();
  ta_get_transaction_object_res_t* obj_res =
      ta_get_transaction_object_res_new();
  if (hash_res == NULL || obj_res == NULL) {
    ret = -1;
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
    flex_trits_to_trytes((tryte_t*)hash_trytes, NUM_TRYTES_HASH, hash_trits,
                         NUM_TRITS_HASH, NUM_TRITS_HASH);
    hash243_queue_pop(&hash_res->hashes);
    hash_trytes[NUM_TRYTES_HASH] = '\0';
    ret = ta_get_transaction_object(service, hash_trytes, obj_res);
    if (ret) {
      break;
    }
    utarray_push_back(res->txn_obj, obj_res->txn);
  }

done:
  ta_find_transactions_res_free(&hash_res);
  ta_get_transaction_object_res_free(&obj_res);
  return ret;
}
