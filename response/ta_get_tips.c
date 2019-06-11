#include "ta_get_tips.h"

ta_get_tips_res_t* ta_get_tips_res_new() {
  ta_get_tips_res_t* res = (ta_get_tips_res_t*)malloc(sizeof(ta_get_tips_res_t));
  if (res) {
    res->tips = NULL;
  }
  return res;
}

void ta_get_tips_res_free(ta_get_tips_res_t** res) {
  if (*res) {
    hash243_stack_free(&(*res)->tips);
  }
  free(*res);
  *res = NULL;
}
