/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_HTTPS_H
#define UTILS_HTTPS_H

#include "common/defined_error.h"

/**
 * @brief Send message via HTTP(S) protocol
 *
 * @param[in] host HTTP(S) host
 * @param[in] port HTTP(S) port
 * @param[in] api API path for POST request to HTTP(S) server, i.e "transaction/". It must be in string.
 * @param[in] msg Message to send
 * @param[in] msg_len Length of message
 * @param[in] ssl_seed Seed for ssl connection
 *
 * @return #endpoint_retcode_t
 */
endpoint_retcode_t send_https_msg(char const *host, char const *port, char const *api, const char *msg,
                                  const int msg_len, const char *ssl_seed);
#endif  // UTILS_HTTPS_H
