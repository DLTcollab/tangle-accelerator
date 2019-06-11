#include "ta_send_transfer.h"

ta_send_transfer_req_t* ta_send_transfer_req_new() {
  ta_send_transfer_req_t* req = (ta_send_transfer_req_t*)malloc(sizeof(ta_send_transfer_req_t));
  if (req != NULL) {
    req->tag = NULL;
    req->address = NULL;
    return req;
  }
  return NULL;
}

void ta_send_transfer_req_free(ta_send_transfer_req_t** req) {
  if (*req) {
    hash81_queue_free(&(*req)->tag);
    hash243_queue_free(&(*req)->address);
    free(*req);
    *req = NULL;
  }
}
