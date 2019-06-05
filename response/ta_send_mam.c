#include "ta_send_mam.h"

ta_send_mam_res_t* send_mam_res_new() {
  ta_send_mam_res_t* res =
      (ta_send_mam_res_t*)malloc(sizeof(ta_send_mam_res_t));

  return res;
}

status_t send_mam_res_set_bundle_hash(ta_send_mam_res_t* res,
                                      const tryte_t* bundle_hash) {
  if (!bundle_hash || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->bundle_hash, bundle_hash, NUM_TRYTES_HASH);
  res->bundle_hash[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_channel_id(ta_send_mam_res_t* res,
                                     const tryte_t* channel_id) {
  if (!channel_id || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->channel_id, channel_id, NUM_TRYTES_HASH);
  res->channel_id[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

void send_mam_res_free(ta_send_mam_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  free(*res);
  *res = NULL;
}
