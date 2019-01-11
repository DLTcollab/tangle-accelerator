#ifndef COMMON_CORE_H_
#define COMMON_CORE_H_

#include "./config.h"
#include "cclient/iota_client_core_api.h"
#include "cclient/iota_client_extended_api.h"
#include "request/request.h"
#include "response/response.h"
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int cclient_get_txn_to_approve(const iota_client_service_t* const service,
                               ta_get_tips_res_t* res);
int cclient_get_tips(const iota_client_service_t* const service,
                     ta_get_tips_res_t* res);
int ta_generate_address(const iota_client_service_t* const service,
                        ta_generate_address_res_t* res);
int ta_get_tips(const iota_client_service_t* const service,
                const ta_get_tips_req_t* const req, ta_get_tips_res_t* res);
int ta_send_transfer(const iota_client_service_t* const service,
                     const ta_send_transfer_req_t* const req,
                     ta_send_transfer_res_t* res);
int ta_find_transactions_by_tag(const iota_client_service_t* const service,
                                const char* const req,
                                ta_find_transactions_res_t* const res);
int ta_get_transaction_msg(const iota_client_service_t* const service,
                           const ta_get_transaction_msg_req_t* req,
                           ta_get_transaction_msg_res_t* res);

#ifdef __cplusplus
}
#endif

#endif  // COMMON_CORE_H_
