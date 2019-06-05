#ifndef REQUEST_TA_SEND_MAM_H_
#define REQUEST_TA_SEND_MAM_H_

#include "accelerator/errors.h"
#include "cclient/types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct send_mam_req_s {
  tryte_t* payload_trytes;
  size_t payload_trytes_size;
} ta_send_mam_req_t;

ta_send_mam_req_t* send_mam_req_new();
status_t send_mam_req_set_payload(ta_send_mam_req_t* req, const tryte_t* payload);
void send_mam_req_free(ta_send_mam_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_SEND_MAM_H_
