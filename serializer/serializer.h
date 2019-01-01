#ifndef SERIALIZER_SERIALIZER_H_
#define SERIALIZER_SERIALIZER_H_

#include <stdlib.h>

#include "cJSON.h"
#include "request/request.h"
#include "response/response.h"
#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

int ta_generate_address_res_serialize(
    char** obj, const ta_generate_address_res_t* const res);
int ta_get_tips_req_deserialize(const char* const obj, ta_get_tips_req_t* req);
int ta_get_tips_res_serialize(char** obj, const ta_get_tips_res_t* const res);
int ta_send_transfer_req_deserialize(const char* const obj,
                                     ta_send_transfer_req_t* req);
int ta_send_transfer_res_serialize(char** obj,
                                   const ta_send_transfer_res_t* const res);
int ta_get_transaction_msg_req_deserialize(const char* const obj,
                                           ta_get_transaction_msg_req_t* req);
int ta_get_transaction_msg_res_serialize(
    char** obj, const ta_get_transaction_msg_res_t* const res);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_SERIALIZER_H_
