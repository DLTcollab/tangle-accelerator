/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_fetch_buffered_request_status_H_
#define RESPONSE_TA_fetch_buffered_request_status_H_

#include "common/macros.h"
#include "common/model/bundle.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/response/ta_fetch_buffered_request_status.h
 */

/**
 * The enumeration of the status of buffered requests
 */
typedef enum {
  UNSENT /**< Buffered request hasn't be broadcasted */,
  NOT_EXIST /**< A request with given UUID is not existed */,
  SENT /**< Buffered request has be broadcasted */
} request_with_uuid_status_t;

/** struct of ta_fetch_buffered_request_status_res */
typedef struct ta_fetch_buffered_request_status_res_s {
  bundle_transactions_t* bundle;     /**< bundle transaction object. */
  request_with_uuid_status_t status; /**< request status. */
  char* mam_result;                  /**< the result of buffered MAM requests are in string. */
} ta_fetch_buffered_request_status_res_t;

/**
 * @brief Allocate memory of ta_fetch_buffered_request_status_res_t
 *
 * @return
 * - struct of ta_fetch_buffered_request_status_res_t on success
 * - NULL on error
 */
ta_fetch_buffered_request_status_res_t* ta_fetch_buffered_request_status_res_new();

/**
 * @brief Free memory of ta_fetch_buffered_request_status_res_t
 *
 * @param[in] res Response in `ta_fetch_buffered_request_status_res_t` datatype
 */
void ta_fetch_buffered_request_status_res_free(ta_fetch_buffered_request_status_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_FETCH_BUFFERED_REQUEST_STATUS_H_
