#include "ta_send_transfer.h"

ta_send_transfer_res_t* ta_send_transfer_res_new() {
  ta_send_transfer_res_t* res =
      (ta_send_transfer_res_t*)malloc(sizeof(ta_send_transfer_res_t));
  return res;
}

void ta_send_transfer_res_free(ta_send_transfer_res_t* res) {
  if (res) {
    trit_array_free(res->bundle);
    free(res);
    res = NULL;
  }
}
