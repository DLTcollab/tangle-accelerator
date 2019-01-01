#ifndef RESPONSE_TA_GET_TIPS_H_
#define RESPONSE_TA_GET_TIPS_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  hash243_stack_t tips;
} ta_get_tips_res_t;

ta_get_tips_res_t* ta_get_tips_res_new();
void ta_get_tips_res_free(ta_get_tips_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GET_TIPS_H_
