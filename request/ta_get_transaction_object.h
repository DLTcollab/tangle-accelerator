#ifndef REQUEST_TA_GET_TRANSACTION_OBJECT_H_
#define REQUEST_TA_GET_TRANSACTION_OBJECT_H_

#include "utarray.h"
#include "accelerator/errors.h"
#include "common/model/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct{
    UT_array* hashes;
} ta_get_transaction_object_req_t;

ta_get_transaction_object_req_t* ta_get_transaction_object_req_new();
status_t ta_get_transaction_object_req_push_address(ta_get_transaction_object_req_t *req, const char* const addr);
void ta_get_transaction_object_req_free(ta_get_transaction_object_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_GET_TRANSACTION_OBJECT_H_
