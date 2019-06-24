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
    req->prng[0] = 0;
    req->payload = NULL;
    req->channel_ord = 0;
  }

  return req;
}

void send_mam_req_free(ta_send_mam_req_t** req) {
  if (!req || !(*req)) {
    return;
  }

  if ((*req)->payload) {
    free((*req)->payload);
    (*req)->payload = NULL;
  }

  free(*req);
  *req = NULL;
}
