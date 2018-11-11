#ifndef RESPONSE_TA_SEND_TRANSFER_H_
#define RESPONSE_TA_SEND_TRANSFER_H_

#include "types/types.h"

typedef struct {
  trit_array_p bundle;
} ta_send_transfer_res_t;

ta_send_transfer_res_t* ta_send_transfer_res_new();
void ta_send_transfer_res_free(ta_send_transfer_res_t* res);

#endif  // RESPONSE_TA_SEND_TRANSFER_H_
