#ifndef REQUEST_TA_FIND_TRANSACTIONS_H_
#define REQUEST_TA_FIND_TRANSACTIONS_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  hash81_queue_t tags;
} ta_find_transactions_req_t;

ta_find_transactions_req_t* ta_find_transactions_req_new();
void ta_find_transactions_req_free(ta_find_transactions_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_FIND_TRANSACTIONS_H_
