#include "ta_register_mam_channel.h"

ta_register_mam_channel_req_t* ta_register_mam_channel_req_new() {
  ta_register_mam_channel_req_t* req = (ta_register_mam_channel_req_t*)malloc(sizeof(ta_register_mam_channel_req_t));
  if (req != NULL) {
    return req;
  }
  return NULL;
}

void ta_register_mam_channel_req_free(ta_register_mam_channel_req_t** req) {
  if (!req || !(*req)) {
    return;
  }
  free(*req);
  *req = NULL;
}
