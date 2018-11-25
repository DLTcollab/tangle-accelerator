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
                                                &get_txn_res);
  if (ret != RC_OK) {
    get_transactions_to_approve_req_free(&get_txn_req);
    get_transactions_to_approve_res_free(&get_txn_res);
    return -1;
  }

  res = flex_hash_array_append_hash(res, get_txn_res->branch);
  res = flex_hash_array_append_hash(res, get_txn_res->trunk);
  get_txn_res = NULL;
  get_transactions_to_approve_req_free(&get_txn_req);
  return 0;
}

int cclient_get_tips(const iota_client_service_t* const service,
                     ta_get_tips_res_t* res) {
  retcode_t ret = RC_OK;
  get_tips_res_t* get_tips_res = get_tips_res_new();
  ret = iota_client_get_tips(service, get_tips_res);
  if (ret != RC_OK) {
    get_tips_res_free(get_tips_res);
    res = NULL;
    return -1;
  }
  res = get_tips_res->tips;
  get_tips_res = NULL;
  return 0;
}

int insert_to_trytes(const size_t start, const size_t end,
                     const trit_array_p* tryte, const trit_array_p* to_tryte) {
  return 0;
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
    memcpy(res->hash->trits, out_address->hash, sizeof(flex_trit_t) * 243);
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
      res = NULL;
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

int ta_find_transaction_by_tag(const iota_client_service_t* const service,
                               const flex_hash_array_t* const tag) {
  return 0;
}

int ta_get_txn_msg(const flex_hash_array_t* const txn) { return 0; }
