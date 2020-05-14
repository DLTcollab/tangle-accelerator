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
    recv_mam_data_id_mam_v1_t* data_id = (*req)->data_id;
    free(data_id->bundle_hash);
    free(data_id->chid);
    free(data_id->msg_id);
    data_id->bundle_hash = NULL;
    data_id->chid = NULL;
    data_id->msg_id = NULL;

    free((*req)->data_id);
    (*req)->data_id = NULL;
  }

  if ((*req)->key) {
    recv_mam_key_mam_v1_t* key = (*req)->key;
    utarray_free(key->psk_array);
    utarray_free(key->ntru_array);
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

status_t recv_mam_req_v1_init(ta_recv_mam_req_t* req) {
  if (req == NULL) {
    return SC_TA_NULL;
  }

  req->data_id = (recv_mam_data_id_mam_v1_t*)malloc(sizeof(recv_mam_data_id_mam_v1_t));
  if (req->data_id == NULL) {
    return SC_TA_OOM;
  }
  req->key = (recv_mam_key_mam_v1_t*)malloc(sizeof(recv_mam_key_mam_v1_t));
  if (req->key == NULL) {
    return SC_TA_OOM;
  }

  recv_mam_data_id_mam_v1_t* data_id = (recv_mam_data_id_mam_v1_t*)req->data_id;
  data_id->bundle_hash = NULL;
  data_id->chid = NULL;
  data_id->msg_id = NULL;

  recv_mam_key_mam_v1_t* key = (recv_mam_key_mam_v1_t*)req->key;
  utarray_new(key->psk_array, &ut_str_icd);
  utarray_new(key->ntru_array, &ut_str_icd);

  return SC_OK;
}

status_t recv_mam_set_mam_v1_data_id(ta_recv_mam_req_t* req, char* bundle_hash, char* chid, char* msg_id) {
  if (req == NULL || (!bundle_hash && !chid && !msg_id)) {
    return SC_TA_NULL;
  }
  status_t ret = SC_OK;

  recv_mam_data_id_mam_v1_t* data_id = (recv_mam_data_id_mam_v1_t*)req->data_id;
  if (bundle_hash) {
    data_id->bundle_hash = (tryte_t*)strdup(bundle_hash);
    if (!data_id->bundle_hash) {
      ret = SC_TA_OOM;
      goto error;
    }
  }
  if (chid) {
    data_id->chid = (tryte_t*)strdup(chid);
    if (!data_id->chid) {
      ret = SC_TA_OOM;
      goto error;
    }
  }
  if (msg_id) {
    data_id->msg_id = (tryte_t*)strdup(msg_id);
    if (!data_id->msg_id) {
      ret = SC_TA_OOM;
      goto error;
    }
  }

  return ret;

error:
  free(data_id->bundle_hash);
  free(data_id->chid);
  free(data_id->msg_id);
  return ret;
}
