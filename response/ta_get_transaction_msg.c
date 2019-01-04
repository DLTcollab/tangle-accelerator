#include "ta_get_transaction_msg.h"

ta_get_transaction_msg_res_t* ta_get_transaction_msg_res_new() {
  ta_get_transaction_msg_res_t* res = (ta_get_transaction_msg_res_t*)malloc(
      sizeof(ta_get_transaction_msg_res_t));
  return res;
}

void ta_get_transaction_msg_res_free(ta_get_transaction_msg_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  free(*res);
  *res = NULL;
}
