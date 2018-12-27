#include "common_core.h"

flex_hash_array_t* flex_hash_array_append_hash(flex_hash_array_t* head,
                                               trit_array_p hash) {
  flex_hash_array_t* tmp =
      (flex_hash_array_t*)malloc(sizeof(flex_hash_array_t));
  if (hash != NULL) {
    if (tmp != NULL) {
      tmp->hash = hash;
      LL_APPEND(head, tmp);
    } else {
      free(tmp);
    }
  }
  return head;
}

int cclient_get_txn_to_approve(const iota_client_service_t* const service,
                               ta_get_tips_res_t* res) {
  retcode_t ret = RC_OK;
  get_transactions_to_approve_req_t* get_txn_req =
      get_transactions_to_approve_req_new();
  get_transactions_to_approve_res_t* get_txn_res =
      get_transactions_to_approve_res_new();
  get_txn_req->depth = 15;

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
  get_tips_res = NULL;
  return 0;
}

trit_array_p insert_to_trytes(const size_t start, const trit_array_p tryte,
                              const trit_array_p to_tryte) {
  return trit_array_insert(tryte, to_tryte, start * 3, to_tryte->num_trits);
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
  hash243_queue_free(&out_address);
  return ret;
}

int ta_get_tips(const iota_client_service_t* const service,
                const ta_get_tips_req_t* const req, ta_get_tips_res_t* res) {
  if (req == NULL || res == NULL) {
    return -1;
  }
  switch (req->opt) {
    case 0:
      /* get_transactions_to_approve */
      if (cclient_get_txn_to_approve(service, res)) return -1;
    case 1:
      res->tips = NULL;
      break;
    case 2:
      /* get_tips */
      if (cclient_get_tips(service, res)) return -1;
    default:
      /* invalid option */
      return -1;
  }
  return 0;
}

int ta_send_transfer(const iota_client_service_t* const service,
                     const ta_send_transfer_req_t* const req,
                     ta_send_transfer_res_t* res) {
  /* make bundle
   * pow
   * broadcast
   */
  return 0;
}

int ta_attach_debug_message_to_tangle(const char* const msg) { return 0; }

int ta_find_transactions_by_tag(const iota_client_service_t* const service,
                                const ta_find_transactions_req_t* const req,
                                ta_find_transactions_res_t* res) {
  retcode_t ret = RC_OK;
  find_transactions_req_t* find_tx_req = find_transactions_req_new();
  find_transactions_res_t* find_tx_res = find_transactions_res_new();

  find_tx_req->tags = req->tags;
  ret = iota_client_find_transactions(service, find_tx_req, find_tx_res);

  if (ret == RC_OK) {
    res->hashes = find_tx_res->hashes;
    find_tx_res->hashes = NULL;
  }

  find_transactions_req_free(&find_tx_req);
  find_transactions_res_free(&find_tx_res);
  return ret;
}

int ta_get_transaction_msg(const iota_client_service_t* const service,
                           const ta_get_transaction_msg_req_t* req,
                           ta_get_transaction_msg_res_t* res) {
  if (req == NULL || res == NULL) {
    return -1;
  }

  retcode_t ret = RC_OK;
  iota_transaction_t* tx;
  flex_trit_t* tx_trits;

  // get raw transaction data of transaction hashes
  get_trytes_req_t* get_trytes_req = get_trytes_req_new();
  get_trytes_res_t* get_trytes_res = get_trytes_res_new();
  if (get_trytes_req == NULL || get_trytes_res == NULL) {
    goto done;
  }

  get_trytes_req->hashes = req->hashes;
  ret = iota_client_get_trytes(service, get_trytes_req, get_trytes_res);
  if (ret) {
    goto done;
  }

  // deserialize raw data to transaction object and get message
  tx_trits = hash8019_queue_peek(get_trytes_res->trytes);
  if (tx_trits == NULL) {
    goto done;
  }

  tx = transaction_deserialize(tx_trits, 0);
  if (tx == NULL) {
    goto done;
  }

  ret = hash8019_queue_push(&res->msg, transaction_message(tx));
  if (ret) {
    goto done;
  }

done:
  get_trytes_req_free(&get_trytes_req);
  get_trytes_res_free(&get_trytes_res);
  return ret;
}
