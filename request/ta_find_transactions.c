#include "ta_find_transactions.h"

ta_find_transactions_req_t* ta_find_transactions_req_new() {
  ta_find_transactions_req_t* req =
      (ta_find_transactions_req_t*)malloc(sizeof(ta_find_transactions_req_t));
  if (req != NULL) {
    req->tags = NULL;
    return req;
  }
  return NULL;
}

void ta_find_transactions_req_free(ta_find_transactions_req_t* req) {
  if (req->tags) {
    hash81_queue_free(&req->tags);
  }
  free(req);
  req = NULL;
}
