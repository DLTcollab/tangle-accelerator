#ifndef REQUEST_TA_SEND_TRANSFER_H_
#define REQUEST_TA_SEND_TRANSFER_H_

#include "types/types.h"

typedef struct {
  int value;
  trit_array_p tag;
  trit_array_p address;
  trit_array_p message;
  trit_array_p trunk;
  trit_array_p branch;
} ta_send_transfer_req_t;

ta_send_transfer_req_t* ta_send_transfer_req_new();
void ta_send_transfer_req_free(ta_send_transfer_req_t* req);

#endif  // REQUEST_TA_SEND_TRANSFER_H_
