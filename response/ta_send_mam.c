#include "ta_send_mam.h"

send_mam_res_t* send_mam_res_new() {
  send_mam_res_t* res = (send_mam_res_t*)malloc(sizeof(send_mam_res_t));
  if (res) {
    res->bundle_hash = NULL;
    res->channel_id = NULL;
  }

  return res;
}

status_t send_mam_res_set_bundle_hash(send_mam_res_t* res,
                                      const tryte_t* bundle_hash) {
  if (res->bundle_hash || !bundle_hash) {
    return SC_RES_NULL;
  }

  size_t bundle_hash_size = NUM_TRYTES_ADDRESS * sizeof(char);
  res->bundle_hash = (char*)malloc(bundle_hash_size);
  if (!res->bundle_hash) {
    return SC_RES_OOM;
  }

  memcpy(res->bundle_hash, bundle_hash, bundle_hash_size);
  return SC_OK;
}

status_t send_mam_res_set_channel_id(send_mam_res_t* res,
                                     const tryte_t* channel_id) {
  if (res->channel_id || !channel_id) {
    return SC_RES_NULL;
  }

  size_t channel_id_size = NUM_TRYTES_ADDRESS * sizeof(char);
  res->channel_id = (char*)malloc(channel_id_size);
  if (!res->channel_id) {
    return SC_RES_OOM;
  }

  memcpy(res->channel_id, channel_id, channel_id_size);
  return SC_OK;
}

void send_mam_res_free(send_mam_res_t** res) {
  if (!res || !(*res)) {
    return;
  }
  if ((*res)->bundle_hash) {
    free((*res)->bundle_hash);
    (*res)->bundle_hash = NULL;
  }
  if ((*res)->channel_id) {
    free((*res)->channel_id);
    (*res)->channel_id = NULL;
  }

  free(*res);
  *res = NULL;
}
