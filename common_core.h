#ifndef COMMON_CORE_H_
#define COMMON_CORE_H_

#include "entangled/cclient/iota_client_core_api.h"
#include "entangled/cclient/types/types.h"

flex_hash_array_t* flex_hash_array_append_hash(flex_hash_array_p* head,
                                               trit_array_p* hash);
int insert_to_trytes(const int start, const int end,
                     const trit_array_p* str_insert,
                     const trit_array_p* trytes);
int generate_address(const iota_client_service_t* const service,
                     generate_address_res_t* res);
int get_tips(const iota_client_service_t* const service,
             const get_tips_req_t* const req, get_tips_res_t* res);
int send_transfer(const iota_client_service_t* const service,
                  const send_transafer_req_t* const req,
                  send_transfer_res_t* res);
int attach_debug_message_to_tangle(const char* const msg);
int find_transaction_by_tag(const flex_hash_array_t* const tag);
int get_txn_msg(const flex_hash_array_t* const txn);

#endif  // COMMON_CORE_H_
