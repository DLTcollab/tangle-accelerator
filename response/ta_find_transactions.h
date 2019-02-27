#ifndef RESPONSE_TA_FIND_TRANSACTIONS_H_
#define RESPONSE_TA_FIND_TRANSACTIONS_H_

#include "cclient/types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_find_transactions.h
 */

/** struct of ta_find_transactions_res_t */
typedef struct ta_find_transactions_res {
  /** Transaction hashes is a 243 long flex trits hash queue. */
  hash243_queue_t hashes;
} ta_find_transactions_res_t;

/**
 * Allocate memory of ta_find_transactions_res_t
 *
 * @return
 * - struct of ta_find_transactions_res_t on success
 * - NULL on error
 */
ta_find_transactions_res_t* ta_find_transactions_res_new();

/**
 * Free memory of ta_find_transactions_res_t
 *
 * @param res Data type of ta_find_transactions_res_t
 *
 * @return NULL
 */
void ta_find_transactions_res_free(ta_find_transactions_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_FIND_TRANSACTIONS_H_
