/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_HTTPS_H
#define UTILS_HTTPS_H

#include <stddef.h>
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file endpoint/https.h
 */

/* struct type of HTTP(S) response */
typedef struct {
  char* buffer; /**< Message body **/
  size_t len;   /**< Length of message body */
} https_response_t;

/* struct type of HTTP(S) context */
typedef struct {
  const char* host; /**< HTTP(S) host */
  const int port;   /**< HTTP(S) port */
  const char* api; /**< API path for POST or GET request to HTTP(S) server, i.e "transaction/". It must be in string. */
  const char* ssl_seed;    /**< Seed for ssl connection. This column is optional. */
  https_response_t* s_req; /**< [in] The message to send */
  https_response_t* s_res; /**< [out] The message body of HTTP(S) response */
} https_ctx_t;

/**
 * @brief Initialize logger of HTTP(S)
 */
void https_logger_init();

/**
 * @brief Release logger of https
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int https_logger_release();

/**
 * @brief Send a POST request message via HTTP(S) protocol
 *
 * @param[in, out] ctx The pointer points to http context
 *
 * @return #status_t
 */
status_t send_https_msg(https_ctx_t* ctx);

#ifdef __cplusplus
}
#endif
#endif  // UTILS_HTTPS_H
