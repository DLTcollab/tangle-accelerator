#ifndef REQUEST_TA_GET_TRANSACTION_MSG_H_
#define REQUEST_TA_GET_TRANSACTION_MSG_H_

#include "types/types.h"

typedef struct {
  hash243_queue_t hashes;
} ta_get_transaction_msg_req_t;

ta_get_transaction_msg_req_t* ta_get_transaction_msg_req_new();
void ta_get_transaction_msg_req_free(ta_get_transaction_msg_req_t** req);

#endif  // REQUEST_TA_GET_TRANSACTION_MSG_H_
