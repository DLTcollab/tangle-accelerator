#include "ta_generate_address.h"

ta_generate_address_res_t* ta_generate_address_res_new() {
  ta_generate_address_res_t* res = (ta_generate_address_res_t*)malloc(sizeof(ta_generate_address_res_t));
  if (res) {
    res->addresses = NULL;
  }
  return res;
}

void ta_generate_address_res_free(ta_generate_address_res_t** res) {
  if ((*res)) {
    hash243_queue_free(&(*res)->addresses);
  }
  free(*res);
  *res = NULL;
}
