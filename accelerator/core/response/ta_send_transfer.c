/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_send_transfer.h"

ta_send_transfer_res_t* ta_send_transfer_res_new() {
  ta_send_transfer_res_t* res = (ta_send_transfer_res_t*)malloc(sizeof(ta_send_transfer_res_t));
  if (res) {
    res->hash = NULL;
    res->uuid = NULL;
  }
  return res;
}

void ta_send_transfer_res_free(ta_send_transfer_res_t** res) {
  if ((*res)) {
    free((*res)->uuid);
    hash243_queue_free(&(*res)->hash);
    free((*res));
    *res = NULL;
  }
}
