#include "ta_send_mam.h"

send_mam_res_t* send_mam_res_new() {
  send_mam_res_t* res = (send_mam_res_t*)malloc(sizeof(send_mam_res_t));

  return res;
}

status_t send_mam_res_set_bundle_hash(send_mam_res_t* res,
                                      const tryte_t* bundle_hash) {
  if (!bundle_hash || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->bundle_hash, bundle_hash, NUM_TRYTES_HASH);
  res->bundle_hash[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_channel_id(send_mam_res_t* res,
                                     const tryte_t* channel_id) {
  if (!channel_id || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->channel_id, channel_id, NUM_TRYTES_HASH);
  res->channel_id[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

void send_mam_res_free(send_mam_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  free(*res);
  *res = NULL;
}
