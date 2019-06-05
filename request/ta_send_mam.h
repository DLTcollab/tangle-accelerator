#ifndef REQUEST_TA_SEND_MAM_H_
#define REQUEST_TA_SEND_MAM_H_

#include "accelerator/errors.h"
#include "cclient/types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_send_mam.h
 */

/** struct of ta_send_transfer_req_t */
typedef struct send_mam_req_s {
  tryte_t* payload_trytes;
  size_t payload_trytes_size;
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
 * Set payload of ta_send_mam_req_t object
 *
 * @param req Data type of ta_send_transfer_req_t
 * @param payload Tryte data going to send with mam
 *
 * @return NULL
 */
status_t send_mam_req_set_payload(ta_send_mam_req_t* req,
                                  const tryte_t* payload);

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
