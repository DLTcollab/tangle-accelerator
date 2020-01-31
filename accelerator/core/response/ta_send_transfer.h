/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_SEND_TRANSFER_H_
#define RESPONSE_TA_SEND_TRANSFER_H_

#include <stdlib.h>
#ifdef DB_ENABLE
#include "storage/ta_storage.h"
#endif
#include "common/model/transaction.h"
#include "utils/containers/hash/hash243_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/response/ta_send_transfer.h
 */

/** struct of ta_send_transfer_res_t */
typedef struct {
  /** Transaction address is a 243 long flex trits hash queue. */
  hash243_queue_t hash;
  transaction_array_t* txn_array;
#ifdef DB_ENABLE
  char uuid_string[DB_UUID_STRING_LENGTH];
#endif
} ta_send_transfer_res_t;

/**
 * Allocate memory of ta_send_transfer_res_t
 *
 * @return
 * - struct of ta_send_transfer_res_t on success
 * - NULL on error
 */
ta_send_transfer_res_t* ta_send_transfer_res_new();

/**
 * Free memory of ta_send_transfer_res_t
 *
 * @param res Data type of ta_send_transfer_res_t
 */
void ta_send_transfer_res_free(ta_send_transfer_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_SEND_TRANSFER_H_
