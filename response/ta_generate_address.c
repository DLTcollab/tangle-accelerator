#include "ta_generate_address.h"

ta_generate_address_res_t* ta_generate_address_res_new() {
  /* for utlist macro, head must be initialize to NULL*/
  return NULL;
}

void ta_generate_address_res_free(ta_generate_address_res_t* res) {
  flex_hash_array_free(res);
}
