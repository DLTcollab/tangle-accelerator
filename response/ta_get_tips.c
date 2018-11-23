#include "ta_get_tips.h"

ta_get_tips_res_t* ta_get_tips_res_new() {
  /* for utlist macro, head must be initialize to NULL*/
  return NULL;
}

void ta_get_tips_res_free(ta_get_tips_res_t* res) { flex_hash_array_free(res); }
