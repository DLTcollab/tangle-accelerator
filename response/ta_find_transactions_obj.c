/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_find_transactions_obj.h"

static UT_icd txn_icd = {sizeof(iota_transaction_t), 0, 0};

ta_find_transactions_obj_res_t* ta_find_transactions_obj_res_new() {
  ta_find_transactions_obj_res_t* res = (ta_find_transactions_obj_res_t*)malloc(sizeof(ta_find_transactions_obj_res_t));
  if (res) {
    utarray_new(res->txn_obj, &txn_icd);
  }
  return res;
}

void ta_find_transactions_obj_res_free(ta_find_transactions_obj_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  if ((*res)->txn_obj) {
    utarray_clear((*res)->txn_obj);
    utarray_free((*res)->txn_obj);
  }
  free(*res);
  *res = NULL;
}
