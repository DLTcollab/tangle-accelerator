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
    free((*req)->data_id);
    (*req)->data_id = NULL;
  }

  key_mam_v1_t* key = (*req)->key;
  if (key) {
    if (key->psk) {
      free(key->psk);
    }
    if (key->ntru_pk) {
      free(key->ntru_pk);
    }
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

  req->data_id = (data_id_mam_v1_t*)malloc(sizeof(data_id_mam_v1_t));
  if (!req->data_id) {
    return SC_TA_OOM;
  }

  data_id_mam_v1_t* data_id = (data_id_mam_v1_t*)req->data_id;
  if (bundle_hash) {
    strncpy(data_id->bundle_hash, bundle_hash, NUM_TRYTES_HASH);
  }
  if (chid) {
    strncpy(data_id->chid, chid, NUM_TRYTES_HASH);
  }
  if (epid) {
    strncpy(data_id->epid, epid, NUM_TRYTES_HASH);
  }
  if (msg_id) {
    strncpy(data_id->msg_id, msg_id, NUM_TRYTES_MAM_MSG_ID);
  }

  return SC_OK;
}

status_t set_mam_v1_key(ta_recv_mam_req_t* req, tryte_t* psk, tryte_t* ntru) {
  if (!req->key) {
    return SC_TA_NULL;
  }

  key_mam_v1_t* key = (key_mam_v1_t*)req->key;
  // We will set either psk or ntru, so initializing both fields will avoid errors in the future.
  key->psk = NULL;
  key->ntru_pk = NULL;

  if (psk) {
    key->psk = (tryte_t*)malloc(sizeof(tryte_t) * NUM_TRYTES_MAM_PSK_KEY_SIZE);
    if (!key->psk) {
      return SC_TA_OOM;
    }
    memcpy(key->psk, psk, sizeof(tryte_t) * NUM_TRYTES_MAM_PSK_KEY_SIZE);
  } else if (ntru) {
    key->ntru_pk = (tryte_t*)malloc(sizeof(tryte_t) * NUM_TRYTES_MAM_NTRU_PK_SIZE);
    if (!key->ntru_pk) {
      return SC_TA_OOM;
    }
    memcpy(key->ntru_pk, ntru, sizeof(tryte_t) * NUM_TRYTES_MAM_NTRU_PK_SIZE);
  }

  return SC_OK;
}
