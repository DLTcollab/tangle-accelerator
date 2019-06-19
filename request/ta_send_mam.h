#ifndef REQUEST_TA_SEND_MAM_H_
#define REQUEST_TA_SEND_MAM_H_

#include <stdlib.h>
#include <string.h>
#include "accelerator/errors.h"
#include "common/trinary/tryte.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_send_mam.h
 */

/** struct of ta_send_transfer_req_t */
typedef struct send_mam_req_s {
  char* payload;
  uint8_t channel_ord;
} ta_send_mam_req_t;

/**
 * Allocate memory of ta_send_mam_req_t
 *
 * @return
 * - struct of ta_send_mam_req_t on success
 * - NULL on error
 */
ta_send_mam_req_t* send_mam_req_new();

/**
 * Free memory of ta_send_mam_req_t
 *
 * @param req Data type of ta_send_mam_req_t
 *
 * @return NULL
 */
void send_mam_req_free(ta_send_mam_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_SEND_MAM_H_
