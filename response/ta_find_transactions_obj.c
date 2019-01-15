#include "ta_find_transactions_obj.h"

UT_icd txn_icd = {sizeof(iota_transaction_t), 0, 0};

ta_find_transactions_obj_res_t* ta_find_transactions_obj_res_new() {
  ta_find_transactions_obj_res_t* res = (ta_find_transactions_obj_res_t*)malloc(
      sizeof(ta_find_transactions_obj_res_t));
  if (res) {
    utarray_new(res->txn_obj, &txn_icd);
  }
  return res;
}

void ta_find_transactions_obj_res_free(ta_find_transactions_obj_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  if ((*res)->txn_obj) {
    utarray_clear((*res)->txn_obj);
    utarray_free((*res)->txn_obj);
  }
  free(*res);
  *res = NULL;
}
