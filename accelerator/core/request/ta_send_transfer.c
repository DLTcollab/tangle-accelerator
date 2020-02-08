/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_send_transfer.h"

ta_send_transfer_req_t* ta_send_transfer_req_new() {
  ta_send_transfer_req_t* req = (ta_send_transfer_req_t*)malloc(sizeof(ta_send_transfer_req_t));
  if (req != NULL) {
    req->tag = NULL;
    req->address = NULL;
    return req;
  }
  return NULL;
}

void ta_send_transfer_req_free(ta_send_transfer_req_t** req) {
  if (*req) {
    hash81_queue_free(&(*req)->tag);
    hash243_queue_free(&(*req)->address);
    free(*req);
    *req = NULL;
  }
}
