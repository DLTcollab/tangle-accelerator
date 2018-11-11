#ifndef RESPONSE_GENERATE_ADDRESS_H_
#define RESPONSE_GENERATE_ADDRESS_H_

#include "types/types.h"

typedef struct {
  trit_array_p hash;
} ta_generate_address_res_t;

ta_generate_address_res_t* ta_generate_address_res_new();
void ta_generate_address_res_free(ta_generate_address_res_t* res);

#endif  // RESPONSE_GENERATE_ADDRESS_H_
