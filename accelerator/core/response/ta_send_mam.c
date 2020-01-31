/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_send_mam.h"

ta_send_mam_res_t* send_mam_res_new() {
  ta_send_mam_res_t* res = (ta_send_mam_res_t*)malloc(sizeof(ta_send_mam_res_t));
  res->announcement_bundle_hash[0] = 0;
  res->chid1[0] = 0;
  return res;
}

status_t send_mam_res_set_bundle_hash(ta_send_mam_res_t* res, const tryte_t* bundle_hash) {
  if (!bundle_hash || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->bundle_hash, bundle_hash, NUM_TRYTES_HASH);
  res->bundle_hash[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_channel_id(ta_send_mam_res_t* res, const tryte_t* channel_id) {
  if (!channel_id || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->chid, channel_id, NUM_TRYTES_HASH);
  res->chid[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_endpoint_id(ta_send_mam_res_t* res, const tryte_t* endpoint_id) {
  if (!endpoint_id || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->epid, endpoint_id, NUM_TRYTES_HASH);
  res->epid[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_msg_id(ta_send_mam_res_t* res, const tryte_t* msg_id) {
  if (!msg_id || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->msg_id, msg_id, NUM_TRYTES_MAM_MSG_ID);
  res->msg_id[NUM_TRYTES_MAM_MSG_ID] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_announcement_bundle_hash(ta_send_mam_res_t* res, const tryte_t* announcement_bundle_hash) {
  if (!announcement_bundle_hash || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->announcement_bundle_hash, announcement_bundle_hash, NUM_TRYTES_HASH);
  res->announcement_bundle_hash[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

status_t send_mam_res_set_chid1(ta_send_mam_res_t* res, const tryte_t* chid1) {
  if (!chid1 || !res) {
    return SC_RES_NULL;
  }

  memcpy(res->chid1, chid1, NUM_TRYTES_HASH);
  res->chid1[NUM_TRYTES_HASH] = '\0';
  return SC_OK;
}

void send_mam_res_free(ta_send_mam_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  free(*res);
  *res = NULL;
}
