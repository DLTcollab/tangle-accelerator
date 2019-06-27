/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "iota_api_mock.hh"

extern APIMock APIMockObj;

retcode_t iota_client_get_transactions_to_approve(iota_client_service_t const* const service,
                                                  get_transactions_to_approve_req_t const* const req,
                                                  get_transactions_to_approve_res_t* res) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243], hash_trits_2[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  flex_trits_from_trytes(hash_trits_2, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_2, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  get_transactions_to_approve_res_set_trunk(res, hash_trits_1);
  get_transactions_to_approve_res_set_branch(res, hash_trits_2);
  return APIMockObj.iota_client_get_transactions_to_approve(service, req, res);
}

retcode_t iota_client_get_tips(iota_client_service_t const* const service, get_tips_res_t* res) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_stack_push(&res->hashes, hash_trits_1);
  return APIMockObj.iota_client_get_tips(service, res);
}

retcode_t iota_client_find_transactions(iota_client_service_t const* const service,
                                        find_transactions_req_t const* const req, find_transactions_res_t* res) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(&res->hashes, hash_trits_1);
  return APIMockObj.iota_client_find_transactions(service, req, res);
}

retcode_t iota_client_find_transaction_objects(iota_client_service_t const* const service,
                                               find_transactions_req_t const* const req, transaction_array_t* tx_objs) {
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  iota_transaction_t tx;

  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  transaction_deserialize_from_trits(&tx, tx_trits, false);
  transaction_array_push_back(tx_objs, &tx);
  return APIMockObj.iota_client_find_transaction_objects(service, req, tx_objs);
}

retcode_t iota_client_get_new_address(iota_client_service_t const* const serv, flex_trit_t const* const seed,
                                      address_opt_t const addr_opt, hash243_queue_t* out_addresses) {
  flex_trit_t hash_trits_1[FLEX_TRIT_SIZE_243];
  flex_trits_from_trytes(hash_trits_1, NUM_TRITS_HASH, (const tryte_t*)TRYTES_81_1, NUM_TRYTES_HASH, NUM_TRYTES_HASH);
  hash243_queue_push(out_addresses, hash_trits_1);
  return APIMockObj.iota_client_get_new_address(serv, seed, addr_opt, out_addresses);
}

retcode_t iota_client_get_trytes(iota_client_service_t const* const service, get_trytes_req_t const* const req,
                                 get_trytes_res_t* res) {
  flex_trit_t tx_trits[FLEX_TRIT_SIZE_8019];
  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_1,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);
  hash8019_queue_push(&res->trytes, tx_trits);
  return APIMockObj.iota_client_get_trytes(service, req, res);
}

retcode_t iota_client_get_transaction_objects(iota_client_service_t const* const serv,
                                              get_trytes_req_t* const tx_hashes, transaction_array_t* out_tx_objs) {
  flex_trit_t tx_trits[NUM_TRITS_SERIALIZED_TRANSACTION];
  flex_trit_t hash_trits[NUM_TRITS_HASH];
  flex_trits_from_trytes(tx_trits, NUM_TRITS_SERIALIZED_TRANSACTION, (const tryte_t*)TRYTES_2673_2,
                         NUM_TRYTES_SERIALIZED_TRANSACTION, NUM_TRYTES_SERIALIZED_TRANSACTION);

  iota_transaction_t* temp = transaction_deserialize(tx_trits, true);
  transaction_array_push_back(out_tx_objs, temp);
  transaction_free(temp);

  return APIMockObj.iota_client_get_transaction_objects(serv, tx_hashes, out_tx_objs);
}

status_t ta_send_trytes(iota_config_t const* const tangle, const iota_client_service_t* const service,
                        hash8019_array_p trytes) {
  return APIMockObj.ta_send_trytes(tangle, service, trytes);
}
