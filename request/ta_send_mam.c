/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_send_mam.h"

ta_send_mam_req_t* send_mam_req_new() {
  ta_send_mam_req_t* req = (ta_send_mam_req_t*)malloc(sizeof(ta_send_mam_req_t));
  if (req) {
    req->seed = NULL;
    req->message = NULL;
    req->channel_ord = 0;
    req->ch_mss_depth = 6;
    req->ep_mss_depth = 6;
    req->psk = NULL;
    req->ntru_pk = NULL;
  }

  return req;
}

void send_mam_req_free(ta_send_mam_req_t** req) {
  if (!req || !(*req)) {
    return;
  }

  if ((*req)->seed) {
    free((*req)->seed);
    (*req)->seed = NULL;
  }

  if ((*req)->message) {
    free((*req)->message);
    (*req)->message = NULL;
  }

  if ((*req)->ntru_pk) {
    free((*req)->ntru_pk);
    (*req)->ntru_pk = NULL;
  }

  if ((*req)->psk) {
    free((*req)->psk);
    (*req)->psk = NULL;
  }

  free(*req);
  *req = NULL;
}
