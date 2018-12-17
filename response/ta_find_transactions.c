#include "ta_find_transactions.h"

ta_find_transactions_res_t* ta_find_transactions_res_new() {
  /* for utlist macro, head must be initialize to NULL*/
  return NULL;
}

void ta_find_transactions_res_free(ta_find_transactions_res_t* res) {
  hash243_queue_free(&res);
}
