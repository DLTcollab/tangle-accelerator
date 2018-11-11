#ifndef REQUEST_SEND_TRANSFER_H_
#define REQUEST_SEND_TRANSFER_H_

#include "types/types.h"

typedef struct {
  int values;
  flex_hash_array_t* tags;
  flex_hash_array_t* address;
  flex_hash_array_t* messages;
  trit_array_p* trunk;
  trit_array_p* branch;
} ta_send_transfer_req_t;

ta_send_transfer_req_t* ta_send_transfer_req_new();
void ta_send_transfer_req_free(ta_send_transfer_req_t* req);

#endif  // REQUEST_SEND_TRANSFER_H_
