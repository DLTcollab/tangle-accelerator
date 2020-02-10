/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_recv_mam.h"

ta_recv_mam_req_t* recv_mam_req_new() {
  ta_recv_mam_req_t* req = (ta_recv_mam_req_t*)malloc(sizeof(ta_recv_mam_req_t));
  if (req) {
    req->data_id = NULL;
    req->key = NULL;
  }

  return req;
}

static void recv_mam_req_v1_free(ta_recv_mam_req_t** req) {
  if ((*req)->data_id) {
    data_id_mam_v1_t* data_id = (*req)->data_id;
    free(data_id->bundle_hash);
    free(data_id->chid);
    free(data_id->epid);
    free(data_id->msg_id);
    data_id->bundle_hash = NULL;
    data_id->chid = NULL;
    data_id->epid = NULL;
    data_id->msg_id = NULL;

    free((*req)->data_id);
    (*req)->data_id = NULL;
  }

  if ((*req)->key) {
    key_mam_v1_t* key = (*req)->key;
    free(key->enc_key);
    free(key);
    (*req)->key = NULL;
  }
}

void recv_mam_req_free(ta_recv_mam_req_t** req) {
  if (!req || !(*req)) {
    return;
  }

  switch ((*req)->protocol) {
    case MAM_V1:
      recv_mam_req_v1_free(req);
      break;

    default:
      break;
  }

  free(*req);
  *req = NULL;
}

status_t set_mam_v1_data_id(ta_recv_mam_req_t* req, char* bundle_hash, char* chid, char* epid, char* msg_id) {
  if (req == NULL || (!bundle_hash && !chid && !epid && !msg_id)) {
    return SC_TA_NULL;
  }
  status_t ret = SC_OK;

  req->data_id = (data_id_mam_v1_t*)malloc(sizeof(data_id_mam_v1_t));
  if (!req->data_id) {
    return SC_TA_OOM;
  }

  data_id_mam_v1_t* data_id = (data_id_mam_v1_t*)req->data_id;
  data_id->bundle_hash = NULL;
  data_id->chid = NULL;
  data_id->epid = NULL;
  data_id->msg_id = NULL;
  if (bundle_hash) {
    data_id->bundle_hash = strdup(bundle_hash);
    if (!data_id->bundle_hash) {
      ret = SC_TA_OOM;
      goto error;
    }
  }
  if (chid) {
    data_id->chid = strdup(chid);
    if (!data_id->chid) {
      ret = SC_TA_OOM;
      goto error;
    }
  }
  if (epid) {
    data_id->epid = strdup(epid);
    if (!data_id->epid) {
      ret = SC_TA_OOM;
      goto error;
    }
  }
  if (msg_id) {
    data_id->msg_id = strdup(msg_id);
    if (!data_id->msg_id) {
      ret = SC_TA_OOM;
      goto error;
    }
  }

  return ret;

error:
  free(req->data_id);
  req->data_id = NULL;
  return ret;
}

status_t set_mam_v1_key(ta_recv_mam_req_t* req, tryte_t* psk, tryte_t* ntru) {
  if (!req) {
    return SC_TA_NULL;
  }

  req->key = (key_mam_v1_t*)malloc(sizeof(key_mam_v1_t));
  if (!req->key) {
    return SC_TA_OOM;
  }

  key_mam_v1_t* key = (key_mam_v1_t*)req->key;
  // We will set either psk or ntru, so initializing both fields will avoid errors in the future.
  key->enc_key = NULL;

  if (psk) {
    key->enc_key = (tryte_t*)malloc(sizeof(tryte_t) * NUM_TRYTES_MAM_PSK_KEY_SIZE);
    if (!key->enc_key) {
      return SC_TA_OOM;
    }
    memcpy(key->enc_key, psk, sizeof(tryte_t) * NUM_TRYTES_MAM_PSK_KEY_SIZE);
  } else if (ntru) {
    key->enc_key = (tryte_t*)malloc(sizeof(tryte_t) * NUM_TRYTES_MAM_NTRU_PK_SIZE);
    if (!key->enc_key) {
      return SC_TA_OOM;
    }
    memcpy(key->enc_key, ntru, sizeof(tryte_t) * NUM_TRYTES_MAM_NTRU_PK_SIZE);
  }

  return SC_OK;
}
