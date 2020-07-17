/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_HTTPS_H
#define UTILS_HTTPS_H

#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file endpoint/https.h
 */

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
 * @brief Send message via HTTP(S) protocol
 *
 * @param[in] host HTTP(S) host
 * @param[in] port HTTP(S) port
 * @param[in] api API path for POST request to HTTP(S) server, i.e "transaction/". It must be in string.
 * @param[in] msg Message to send
 * @param[in] ssl_seed Seed for ssl connection
 *
 * @return #status_t
 */
status_t send_https_msg(char const *host, char const *port, char const *api, const char *msg, const char *ssl_seed);

#ifdef __cplusplus
}
#endif
#endif  // UTILS_HTTPS_H
