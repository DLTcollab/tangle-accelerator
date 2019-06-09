#ifndef RESPONSE_TA_GET_TRANSACTION_OBJECT_H_
#define RESPONSE_TA_GET_TRANSACTION_OBJECT_H_

#include "common/model/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_get_transaction_object.h
 */

/** struct of ta_get_transaction_object_res_t */
typedef struct ta_get_transaction_object_res {
  /** Transaction is a iota_transaction_t list. */
  iota_transaction_t* txn;
} ta_get_transaction_object_res_t;

/**
 * Allocate memory of ta_get_transaction_object_res_t
 *
 * @return
 * - struct of ta_get_transaction_object_res_t on success
 * - NULL on error
 */
ta_get_transaction_object_res_t* ta_get_transaction_object_res_new();

/**
 * Free memory of ta_get_transaction_object_res_t
 *
 * @param res Data type of ta_get_transaction_object_res_t
 *
 * @return NULL
 */
void ta_get_transaction_object_res_free(ta_get_transaction_object_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GET_TRANSACTION_OBJECT_H_
