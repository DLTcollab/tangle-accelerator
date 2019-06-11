#include "ta_send_transfer.h"

ta_send_transfer_res_t* ta_send_transfer_res_new() {
  ta_send_transfer_res_t* res = (ta_send_transfer_res_t*)malloc(sizeof(ta_send_transfer_res_t));
  res->hash = NULL;
  return res;
}

void ta_send_transfer_res_free(ta_send_transfer_res_t** res) {
  if ((*res)) {
    hash243_queue_free(&(*res)->hash);
    free((*res));
    *res = NULL;
  }
}
