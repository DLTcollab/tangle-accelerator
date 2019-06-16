#include "ta_get_transaction_object.h"

ta_get_transaction_object_res_t* ta_get_transaction_object_res_new() {
  ta_get_transaction_object_res_t* res =
      (ta_get_transaction_object_res_t*)malloc(sizeof(ta_get_transaction_object_res_t));
  if (res) {
    res->txn_array = transaction_array_new();
  }
  return res;
}

void ta_get_transaction_object_res_free(ta_get_transaction_object_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  if ((*res)->txn_array) {
    transaction_array_free((*res)->txn_array);
  }
  free(*res);
  *res = NULL;
}
