#ifndef COMMON_CORE_H_
#define COMMON_CORE_H_

#include "./config.h"
#include "cclient/iota_client_core_api.h"
#include "cclient/iota_client_extended_api.h"
#include "request/request.h"
#include "response/response.h"
#include "types/types.h"

flex_hash_array_t* flex_hash_array_append_hash(flex_hash_array_t* head,
                                               trit_array_p hash);
int cclient_get_txn_to_approve(const iota_client_service_t* const service,
                               ta_get_tips_res_t* res);
int cclient_get_tips(const iota_client_service_t* const service,
                     ta_get_tips_res_t* res);
int insert_to_trytes(const size_t start, const size_t end,
                     const trit_array_p* tryte, const trit_array_p* to_tryte);
int ta_generate_address(const iota_client_service_t* const service,
                        ta_generate_address_res_t* res);
int ta_get_tips(const iota_client_service_t* const service,
                const ta_get_tips_req_t* const req, ta_get_tips_res_t* res);
int ta_send_transfer(const iota_client_service_t* const service,
                     const ta_send_transfer_req_t* const req,
                     ta_send_transfer_res_t* res);
int ta_attach_debug_message_to_tangle(const char* const msg);
int ta_find_transaction_by_tag(const iota_client_service_t* const service,
                               const flex_hash_array_t* const tag);
int ta_get_txn_msg(const flex_hash_array_t* const txn);

#endif  // COMMON_CORE_H_
