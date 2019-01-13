#ifndef RESPONSE_TA_GET_TRANSACTION_OBJECT_H_
#define RESPONSE_TA_GET_TRANSACTION_OBJECT_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ta_get_transaction_object_res {
  iota_transaction_t* txn;
} ta_get_transaction_object_res_t;

ta_get_transaction_object_res_t* ta_get_transaction_object_res_new();
void ta_get_transaction_object_res_free(ta_get_transaction_object_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GET_TRANSACTION_OBJECT_H_
