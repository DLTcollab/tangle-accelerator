#include "ta_find_transactions.h"

ta_find_transactions_res_t* ta_find_transactions_res_new() {
  /* for utlist macro, head must be initialized to NULL*/
  return NULL;

  ta_find_transactions_res_t* res =
      (ta_find_transactions_res_t*)malloc(sizeof(ta_find_transactions_res_t));
  if (res) {
    res->hashes = NULL;
  }
  return res;
}

void ta_find_transactions_res_free(ta_find_transactions_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  if ((*res)->hashes) {
    hash243_queue_free(&(*res)->hashes);
  }
  free(*res);
  *res = NULL;
}
