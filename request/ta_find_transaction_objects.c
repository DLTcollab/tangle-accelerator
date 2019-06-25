/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_find_transaction_objects.h"

ta_find_transaction_objects_req_t* ta_find_transaction_objects_req_new() {
  ta_find_transaction_objects_req_t* req =
      (ta_find_transaction_objects_req_t*)malloc(sizeof(ta_find_transaction_objects_req_t));
  if (req != NULL) {
    req->hashes = NULL;
    return req;
  }
  return NULL;
}

void ta_find_transaction_objects_req_free(ta_find_transaction_objects_req_t** req) {
  hash243_queue_free(&(*req)->hashes);
  free((*req));
  *req = NULL;
}
