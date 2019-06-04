#ifndef RESPONSE_TA_FIND_TRANSACTIONS_OBJ_H_
#define RESPONSE_TA_FIND_TRANSACTIONS_OBJ_H_

#include "./utarray.h"
#include "common/model/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_find_transactions_obj.h
 */

/** struct of ta_find_transactions_obj_res_t */
typedef struct ta_find_transactions_obj_res {
  /** Transaction objects is a iota_transaction_t UT_array. */
  UT_array* txn_obj;
} ta_find_transactions_obj_res_t;

/**
 * Allocate memory of ta_find_transactions_obj_res_t
 *
 * @return
 * - struct of ta_find_transactions_obj_res_t on success
 * - NULL on error
 */
ta_find_transactions_obj_res_t* ta_find_transactions_obj_res_new();

/**
 * Free memory of ta_find_transactions_obj_res_t
 *
 * @param res Data type of ta_find_transactions_obj_res_t
 *
 * @return NULL
 */
void ta_find_transactions_obj_res_free(ta_find_transactions_obj_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_FIND_TRANSACTIONS_OBJ_H_
