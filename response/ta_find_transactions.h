#ifndef RESPONSE_TA_FIND_TRANSACTIONS_H_
#define RESPONSE_TA_FIND_TRANSACTIONS_H_

#include "types/types.h"

typedef struct ta_find_transactions_res {
  hash243_queue_t hashes;
} ta_find_transactions_res_t;

ta_find_transactions_res_t* ta_find_transactions_res_new();
void ta_find_transactions_res_free(ta_find_transactions_res_t** res);

#endif  // RESPONSE_TA_FIND_TRANSACTIONS_H_
