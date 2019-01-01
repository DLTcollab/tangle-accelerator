#ifndef RESPONSE_TA_GENERATE_ADDRESS_H_
#define RESPONSE_TA_GENERATE_ADDRESS_H_

#include "types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  hash243_queue_t addresses;
} ta_generate_address_res_t;

ta_generate_address_res_t* ta_generate_address_res_new();
void ta_generate_address_res_free(ta_generate_address_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GENERATE_ADDRESS_H_
