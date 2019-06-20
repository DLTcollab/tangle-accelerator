#include "ta_send_mam.h"

ta_send_mam_req_t* send_mam_req_new() {
  ta_send_mam_req_t* req = (ta_send_mam_req_t*)malloc(sizeof(ta_send_mam_req_t));
  if (req) {
    req->payload = NULL;
    req->channel_ord = 0;
  }

  return req;
}

void send_mam_req_free(ta_send_mam_req_t** req) {
  if (!req || !(*req)) {
    return;
  }

  if ((*req)->payload) {
    free((*req)->payload);
    (*req)->payload = NULL;
  }

  free(*req);
  *req = NULL;
}
