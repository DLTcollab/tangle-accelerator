#include "ta_get_transaction_object.h"

ta_get_transaction_object_res_t* ta_get_transaction_object_res_new() {
  ta_get_transaction_object_res_t* res =
      (ta_get_transaction_object_res_t*)malloc(
          sizeof(ta_get_transaction_object_res_t));
  if (res) {
    res->txn = NULL;
  }
  return res;
}

void ta_get_transaction_object_res_free(ta_get_transaction_object_res_t** res) {
  if (!res || !(*res) || !(*res)->txn) {
    return;
  }

  transaction_free((*res)->txn);
  free(*res);
  *res = NULL;
}
