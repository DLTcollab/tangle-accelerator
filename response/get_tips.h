#ifndef RESPONSE_GET_TIPS_H_
#define RESPONSE_GET_TIPS_H_

#include "types/types.h"

typedef flex_hash_array_t ta_get_tips_res_t;

ta_get_tips_res_t* ta_get_tips_res_new();
void ta_get_tips_res_free(ta_get_tips_res_t* res);

#endif  // RESPONSE_GET_TIPS_H_
