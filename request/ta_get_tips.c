#include "ta_get_tips.h"

ta_get_tips_req_t* ta_get_tips_req_new() {
  ta_get_tips_req_t* req =
      (ta_get_tips_req_t*)malloc(sizeof(ta_get_tips_req_t));
  if (req != NULL) {
    return req;
  }
  return NULL;
}

void ta_get_tips_req_free(ta_get_tips_req_t* req) {
  free(req);
  req = NULL;
}
