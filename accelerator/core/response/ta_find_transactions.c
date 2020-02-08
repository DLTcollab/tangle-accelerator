/*
 * Copyright (C) 2018 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_find_transactions.h"

ta_find_transactions_by_tag_res_t* ta_find_transactions_res_new() {
  ta_find_transactions_by_tag_res_t* res =
      (ta_find_transactions_by_tag_res_t*)malloc(sizeof(ta_find_transactions_by_tag_res_t));
  if (res) {
    res->hashes = NULL;
  }
  return res;
}

void ta_find_transactions_res_free(ta_find_transactions_by_tag_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  if ((*res)->hashes) {
    hash243_queue_free(&(*res)->hashes);
  }
  free(*res);
  *res = NULL;
}
