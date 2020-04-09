/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_FETCH_TXN_WITH_UUID_H_
#define RESPONSE_TA_FETCH_TXN_WITH_UUID_H_

#include "common/model/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/response/ta_fetch_txn_with_uuid.h
 */
typedef enum { UNSENT, NOT_EXIST, SENT } txn_with_uuid_status_t;

/** struct of ta_fetch_txn_with_uuid_res */
typedef struct ta_fetch_txn_with_uuid_res {
  /** Transaction object. */
  iota_transaction_t* txn;
  txn_with_uuid_status_t status;
} ta_fetch_txn_with_uuid_res_t;

/**
 * Allocate memory of ta_fetch_txn_with_uuid_res_t
 *
 * @return
 * - struct of ta_fetch_txn_with_uuid_res_t on success
 * - NULL on error
 */
ta_fetch_txn_with_uuid_res_t* ta_fetch_txn_with_uuid_res_new();

/**
 * Free memory of ta_fetch_txn_with_uuid_res_t
 *
 * @param res Data type of ta_fetch_txn_with_uuid_res_t
 */
void ta_fetch_txn_with_uuid_res_free(ta_fetch_txn_with_uuid_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_FETCH_TXN_WITH_UUID_H_
