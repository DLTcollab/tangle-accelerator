#include "ta_get_transaction_msg.h"

ta_get_transaction_msg_req_t* ta_get_transaction_msg_req_new() {
  ta_get_transaction_msg_req_t* req = (ta_get_transaction_msg_req_t*)malloc(
      sizeof(ta_get_transaction_msg_req_t));
  if (req) {
    req->hashes = NULL;
    return req;
  }
  return NULL;
}

void ta_get_transaction_msg_req_free(ta_get_transaction_msg_req_t** req) {
  if (!req || !(*req)) {
    return;
  }

  if ((*req)->hashes) {
    hash243_queue_free(&(*req)->hashes);
  }
  free(*req);
  *req = NULL;
}
