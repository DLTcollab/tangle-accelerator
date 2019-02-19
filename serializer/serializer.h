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

#define DEFAULT_TAG "POWEREDBYTANGLEACCELERATOR9"
#define DEFAULT_ADDRESS                      \
  "POWEREDBYTANGLEACCELERATOR99999999999999" \
  "99999999999999999999999999999999999999999"
// "Powered by tangle-accelerator" in ASCII
#define DEFAULT_MSG "ZBCDKDTCFDTCSCEAQCMDEAHDPCBDVC9DTCRAPCRCRCTC9DTCFDPCHDCDFD"
#define DEFAULT_MSG_LEN 58

int ta_generate_address_res_serialize(
    char** obj, const ta_generate_address_res_t* const res);
int ta_get_tips_res_serialize(char** obj, const ta_get_tips_res_t* const res);
int ta_send_transfer_req_deserialize(const char* const obj,
                                     ta_send_transfer_req_t* req);
int ta_get_transaction_object_res_serialize(
    char** obj, const ta_get_transaction_object_res_t* const res);
int ta_find_transactions_res_serialize(
    char** obj, const ta_find_transactions_res_t* const res);
int ta_find_transactions_obj_res_serialize(
    char** obj, const ta_find_transactions_obj_res_t* const res);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_SERIALIZER_H_
