/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef REQUEST_TA_GET_TRANSACTION_OBJECT_H_
#define REQUEST_TA_GET_TRANSACTION_OBJECT_H_

#include "accelerator/errors.h"
#include "common/model/transaction.h"
#include "utils/containers/hash/hash243_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_find_transaction_objects.h
 */

/** struct of ta_find_transaction_objects_req_t */
typedef struct ta_find_transaction_objects_req {
  /** Transaction hashes in ascii with UT_array. */
  hash243_queue_t hashes;
} ta_find_transaction_objects_req_t;

/**
 * Allocate memory of ta_find_transaction_objects_req_t
 *
 * @return
 * - struct of ta_find_transaction_objects_req_t on success
 * - NULL on error
 */
ta_find_transaction_objects_req_t* ta_find_transaction_objects_req_new();

/**
 * Free memory of ta_find_transaction_objects_req_t
 *
 * @param req Data type of ta_find_transaction_objects_req_t
 */
void ta_find_transaction_objects_req_free(ta_find_transaction_objects_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_GET_TRANSACTION_OBJECT_H_
