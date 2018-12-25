#ifndef RESPONSE_TA_GET_TRANSACTION_MSG_H_
#define RESPONSE_TA_GET_TRANSACTION_MSG_H_

#include "types/types.h"

typedef struct ta_get_transaction_msg_res {
  hash8019_queue_t msg;
} ta_get_transaction_msg_res_t;

ta_get_transaction_msg_res_t* ta_get_transaction_msg_res_new();
void ta_get_transaction_msg_res_free(ta_get_transaction_msg_res_t** res);

#endif  // RESPONSE_TA_GET_TRANSACTION_MSG_H_
