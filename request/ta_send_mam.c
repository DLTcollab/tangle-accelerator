#include "ta_send_mam.h"

ta_send_mam_req_t* send_mam_req_new() {
  ta_send_mam_req_t* req = (ta_send_mam_req_t*)malloc(sizeof(ta_send_mam_req_t));
  if (req) {
    req->payload_trytes = NULL;
  }

  return req;
}

status_t send_mam_req_set_payload(ta_send_mam_req_t* req, const tryte_t* payload) {
  status_t ret = SC_OK;
  size_t payload_size = sizeof(payload) / sizeof(tryte_t);

  if ((!payload) || (payload_size == 0) || ((payload_size * 3) > SIZE_MAX)) {
    return SC_MAM_OOM;
  }

  req->payload_trytes = (tryte_t*)malloc(payload_size * sizeof(tryte_t));
  if (!req->payload_trytes) {
    return SC_MAM_OOM;
  }
  memcpy(req->payload_trytes, payload, payload_size);
  req->payload_trytes_size = payload_size;

  return ret;
}

void send_mam_req_free(ta_send_mam_req_t** req) {
  if (!req || !(*req)) {
    return;
  }
  if ((*req)->payload_trytes) {
    free((*req)->payload_trytes);
    (*req)->payload_trytes = NULL;
  }
  free(*req);
  *req = NULL;
}
