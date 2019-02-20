#ifndef RESPONSE_TA_SEND_TRANSFER_H_
#define RESPONSE_TA_SEND_TRANSFER_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_send_transfer.h
 */

/** struct of ta_send_transfer_res_t */
typedef struct {
  /** Transaction address is a 243 long flex trits hash queue. */
  hash243_queue_t hash;
} ta_send_transfer_res_t;

/**
 * Allocate memory of ta_send_transfer_res_t
 *
 * @return
 * - struct of ta_send_transfer_res_t on success
 * - NULL on error
 */
ta_send_transfer_res_t* ta_send_transfer_res_new();

/**
 * Free memory of ta_send_transfer_res_t
 *
 * @param res Data type of ta_send_transfer_res_t
 *
 * @return NULL
 */
void ta_send_transfer_res_free(ta_send_transfer_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_SEND_TRANSFER_H_
