#ifndef REQUEST_GET_TIPS_H_
#define REQUEST_GET_TIPS_H_

#include <stdlib.h>

typedef struct {
  int opt;
} ta_get_tips_req_t;

ta_get_tips_req_t* ta_get_tips_req_new();
void ta_get_tips_req_free(ta_get_tips_req_t* req);

#endif  // REQUEST_GET_TIPS_H_
