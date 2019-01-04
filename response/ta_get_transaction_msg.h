#ifndef RESPONSE_TA_GET_TRANSACTION_MSG_H_
#define RESPONSE_TA_GET_TRANSACTION_MSG_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ta_get_transaction_msg_res {
  flex_trit_t msg[FLEX_TRIT_SIZE_6561];
} ta_get_transaction_msg_res_t;

ta_get_transaction_msg_res_t* ta_get_transaction_msg_res_new();
void ta_get_transaction_msg_res_free(ta_get_transaction_msg_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GET_TRANSACTION_MSG_H_
