#ifndef RESPONSE_TA_FIND_TRANSACTIONS_OBJ_H_
#define RESPONSE_TA_FIND_TRANSACTIONS_OBJ_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ta_find_transactions_obj_res {
  UT_array* txn_obj;
} ta_find_transactions_obj_res_t;

ta_find_transactions_obj_res_t* ta_find_transactions_obj_res_new();
void ta_find_transactions_obj_res_free(ta_find_transactions_obj_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_FIND_TRANSACTIONS_OBJ_H_
