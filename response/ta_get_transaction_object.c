/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_get_transaction_object.h"

ta_get_transaction_object_res_t* ta_get_transaction_object_res_new() {
  ta_get_transaction_object_res_t* res =
      (ta_get_transaction_object_res_t*)malloc(sizeof(ta_get_transaction_object_res_t));
  if (res) {
    res->txn = NULL;
  }
  return res;
}

void ta_get_transaction_object_res_free(ta_get_transaction_object_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  if ((*res)->txn) {
    transaction_free((*res)->txn);
  }
  free(*res);
  *res = NULL;
}
