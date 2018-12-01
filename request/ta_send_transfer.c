#include "ta_send_transfer.h"

ta_send_transfer_req_t* ta_send_transfer_req_new() {
  ta_send_transfer_req_t* req =
      (ta_send_transfer_req_t*)malloc(sizeof(ta_send_transfer_req_t));
  if (req != NULL) {
    req->trunk = trit_array_new(NUM_TRITS_ADDRESS);
    req->branch = trit_array_new(NUM_TRITS_ADDRESS);
    return req;
  }
  return NULL;
}

void ta_send_transfer_req_free(ta_send_transfer_req_t* req) {
  if (req) {
    trit_array_free(req->tag);
    trit_array_free(req->address);
    trit_array_free(req->message);
    trit_array_free(req->trunk);
    trit_array_free(req->branch);
    free(req);
    req = NULL;
  }
}
