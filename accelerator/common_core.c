#include "common_core.h"
#include "cache/cache.h"
#include <time.h>

int cclient_get_txn_to_approve(const iota_client_service_t* const service,
                               ta_get_tips_res_t* res) {
  retcode_t ret = RC_OK;
  get_transactions_to_approve_req_t* get_txn_req =
      get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res =
      get_transactions_to_approve_res_new();
  // The depth at which Random Walk starts. Mininal is 3, and max is 15.
  get_transactions_to_approve_req_set_depth(get_txn_req, 3);

  ret = iota_client_get_transactions_to_approve(service, get_txn_req,
                                                get_txn_res);
  if (ret != RC_OK) {
    get_transactions_to_approve_req_free(&get_txn_req);
    get_transactions_to_approve_res_free(&get_txn_res);
    return -1;
  }

  hash243_stack_push(&res->tips, get_txn_res->branch);
  hash243_stack_push(&res->tips, get_txn_res->trunk);

  get_transactions_to_approve_res_free(&get_txn_res);
  get_transactions_to_approve_req_free(&get_txn_req);
  return 0;
}

int cclient_get_tips(const iota_client_service_t* const service,
                     ta_get_tips_res_t* res) {
  retcode_t ret = RC_OK;
  get_tips_res_t* get_tips_res = get_tips_res_new();
  ret = iota_client_get_tips(service, get_tips_res);
  if (ret != RC_OK) {
    get_tips_res_free(&get_tips_res);
    res->tips = NULL;
    return -1;
  }
  res->tips = get_tips_res->hashes;
  get_tips_res->hashes = NULL;
  get_tips_res_free(&get_tips_res);
  return 0;
}

int cclient_prepare_transfer(const iota_client_service_t* const service,
                             transfer_t** const transfers,
                             uint32_t const num_transfer,
                             bundle_transactions_t* out_bundle) {
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

int ta_send_trytes(const iota_client_service_t* const service,
                   hash8019_array_p trytes,
                   transaction_array_t out_transactions) {
  retcode_t ret = RC_OK;
  // mwm: 14, depth:3, reference: NULL
  // Use cclient library temporarily
  ret = iota_client_send_trytes(service, trytes, 14, 3, NULL, out_transactions);
  return ret;
}

int ta_generate_address(const iota_client_service_t* const service,
                        ta_generate_address_res_t* res) {
  retcode_t ret = RC_OK;
  hash243_queue_t out_address = NULL;
  flex_trit_t seed_trits[243];
  flex_trits_from_trytes(seed_trits, 243, (const tryte_t*)SEED, 81, 81);
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
                                   req->msg_len, (uint64_t)time(NULL));
  ret = cclient_prepare_transfer(service, transfers, 1, out_bundle);
  if (ret) {
    goto done;
  }

  BUNDLE_FOREACH(out_bundle, txn) {
    serialized_txn = transaction_serialize(txn);
    utarray_insert(raw_tx, serialized_txn, 0);
    free(serialized_txn);
  }

  ret = ta_send_trytes(service, raw_tx, (transaction_array_t)out_bundle);
  if (ret) {
    goto done;
  }

  txn = (iota_transaction_t*)utarray_front(out_bundle);
  hash243_queue_push(&find_tx_req->bundles, transaction_bundle(txn));
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
  retcode_t ret = RC_OK;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();

  flex_trit_t tag_trits[NUM_TRITS_TAG];
  flex_trits_from_trytes(tag_trits, NUM_TRITS_TAG, (const tryte_t*)req,
                         NUM_TRYTES_TAG, NUM_TRYTES_TAG);
  hash81_queue_push(&find_tx_req->tags, tag_trits);
  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);

  if (ret == RC_OK) {
    res->hashes = find_tx_res->hashes;
    find_tx_res->hashes = NULL;
  }

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

  // get raw transaction data of transaction hashes
  cache_t* cache = cache_init();
  get_trytes_req_t* get_trytes_req = get_trytes_req_new();
  get_trytes_res_t* get_trytes_res = get_trytes_res_new();
  if (cache == NULL || get_trytes_req == NULL || get_trytes_res == NULL) {
    goto done;
  }

  // get in cache first, then get from full node if no result in cache
  char cache_value[FLEX_TRIT_SIZE_8019] = {0};
  ret = cache_get(cache, req, cache_value);
  if (!ret) {
    flex_trits_from_trytes(
        tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)cache_value,
        NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  } else {
    flex_trits_from_trytes(hash_trits, NUM_TRITS_HASH, (const tryte_t*)req,
                           NUM_TRYTES_HASH, NUM_TRYTES_HASH);
    hash243_queue_push(&get_trytes_req->hashes, hash_trits);
    ret = iota_client_get_trytes(service, get_trytes_req, get_trytes_res);
    if (ret) {
      goto done;
    }

    // deserialize raw data to transaction object
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

  res->txn = transaction_deserialize(tx_trits, 0);
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
  if (res == NULL) {
    return -1;
  }
  int ret = 0;
  char hash_trytes[NUM_TRYTES_HASH + 1];
  flex_trit_t* hash_trits;

  ta_find_transactions_res_t* hash_res = ta_find_transactions_res_new();
  ta_get_transaction_object_res_t* obj_res =
      ta_get_transaction_object_res_new();
  if (hash_res == NULL || obj_res == NULL) {
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
