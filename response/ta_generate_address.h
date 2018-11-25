#ifndef RESPONSE_TA_GENERATE_ADDRESS_H_
#define RESPONSE_TA_GENERATE_ADDRESS_H_

#include "types/types.h"

typedef flex_hash_array_t ta_generate_address_res_t;

ta_generate_address_res_t* ta_generate_address_res_new();
void ta_generate_address_res_free(ta_generate_address_res_t* res);

#endif  // RESPONSE_TA_GENERATE_ADDRESS_H_
