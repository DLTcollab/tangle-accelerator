#include "iota_api_mock.hh"
extern APIMock APIMockObj;

retcode_t iota_client_get_transactions_to_approve(
    const iota_client_service_t* const service,
    const get_transactions_to_approve_req_t* const req,
    get_transactions_to_approve_res_t* res) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243],
      hash_trits_2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  get_transactions_to_approve_res_set_trunk(res, hash_trits_1);
  get_transactions_to_approve_res_set_branch(res, hash_trits_2);
  return APIMockObj.iota_client_get_transactions_to_approve(service, req, res);
}

retcode_t iota_client_get_tips(const iota_client_service_t* const service,
                               get_tips_res_t* res) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  hash243_stack_push(&res->hashes, hash_trits_1);
  return APIMockObj.iota_client_get_tips(service, res);
}

retcode_t iota_client_find_transactions(
    const iota_client_service_t* const service,
    const find_transactions_req_t* const req, find_transactions_res_t* res) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  hash243_queue_push(&res->hashes, hash_trits_1);
  return APIMockObj.iota_client_find_transactions(service, req, res);
}

retcode_t iota_client_get_new_address(iota_client_service_t const* const serv,
                                      flex_trit_t const* const seed,
                                      address_opt_t const addr_opt,
                                      hash243_queue_t* out_addresses) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH,
                         (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH,
                         NUM_TRYTES_HASH);
  hash243_queue_push(out_addresses, hash_trits_1);
  return APIMockObj.iota_client_get_new_address(serv, seed, addr_opt,
                                                out_addresses);
}

retcode_t iota_client_get_trytes(const iota_client_service_t* const service,
                                 get_trytes_req_t* const req,
                                 get_trytes_res_t* res) {
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trits_from_trytes(
      tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
      NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  hash8019_queue_push(&res->trytes, tx_trits);
  return APIMockObj.iota_client_get_trytes(service, req, res);
}

status_t ta_send_trytes(const iota_client_service_t* const service,
                        hash8019_array_p trytes) {
  return APIMockObj.ta_send_trytes(service, trytes);
}
