#ifndef REQUEST_TA_SEND_TRANSFER_H_
#define REQUEST_TA_SEND_TRANSFER_H_

#include "types/types.h"

typedef struct {
  int value;
  hash81_queue_t tag;
  hash243_queue_t address;
  flex_trit_t message[FLEX_TRIT_SIZE_6561];
} ta_send_transfer_req_t;

ta_send_transfer_req_t* ta_send_transfer_req_new();
void ta_send_transfer_req_free(ta_send_transfer_req_t** req);

#endif  // REQUEST_TA_SEND_TRANSFER_H_
