/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "ta_recv_mam.h"
ta_recv_mam_res_t* recv_mam_res_new() {
  ta_recv_mam_res_t* res = (ta_recv_mam_res_t*)malloc(sizeof(ta_recv_mam_res_t));
  if (res) {
    memset(res->chid1, 0, NUM_TRYTES_ADDRESS + 1);
    utarray_new(res->payload_array, &ut_str_icd);
  }
  return res;
}

void recv_mam_res_free(ta_recv_mam_res_t** res) {
  if (!res || !(*res)) {
    return;
  }

  utarray_free((*res)->payload_array);
  free(*res);
  *res = NULL;
}
