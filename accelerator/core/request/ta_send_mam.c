/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_send_mam.h"

ta_send_mam_req_t* send_mam_req_new() {
  ta_send_mam_req_t* req = (ta_send_mam_req_t*)malloc(sizeof(ta_send_mam_req_t));
  if (req) {
    req->data = NULL;
    req->key = NULL;
  }

  return req;
}

status_t send_mam_req_v1_init(ta_send_mam_req_t* req) {
  if (req == NULL) {
    return SC_TA_NULL;
  }

  memset(req->service_token, 0, SERVICE_TOKEN_LEN + 1);

  req->data = (send_mam_data_mam_v1_t*)malloc(sizeof(send_mam_data_mam_v1_t));
  if (req->data == NULL) {
    return SC_TA_OOM;
  }
  req->key = (send_mam_key_mam_v1_t*)malloc(sizeof(send_mam_key_mam_v1_t));
  if (req->key == NULL) {
    return SC_TA_OOM;
  }

  send_mam_data_mam_v1_t* data = req->data;
  data->seed = NULL;
  data->message = NULL;
  data->ch_mss_depth = 6;
  data->ep_mss_depth = 6;

  send_mam_key_mam_v1_t* key = req->key;
  utarray_new(key->psk_array, &ut_str_icd);
  utarray_new(key->ntru_array, &ut_str_icd);

  return SC_OK;
}

static void send_mam_req_v1_free(ta_send_mam_req_t** req) {
  if ((*req)->data) {
    send_mam_data_mam_v1_t* data = (*req)->data;
    free(data->seed);
    free(data->message);
    free((*req)->data);
    (*req)->data = NULL;
  }

  if ((*req)->key) {
    send_mam_key_mam_v1_t* key = (*req)->key;
    utarray_free(key->psk_array);
    utarray_free(key->ntru_array);
    free((*req)->key);
    (*req)->key = NULL;
  }
}

void send_mam_req_free(ta_send_mam_req_t** req) {
  if (!req || !(*req)) {
    return;
  }

  if ((*req)->protocol == MAM_V1) {
    send_mam_req_v1_free(req);
  }

  free(*req);
  *req = NULL;
}
