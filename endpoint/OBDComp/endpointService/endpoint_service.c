/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "endpoint_core.h"
#include "legato.h"

int endpoint_Send_mam_message(const char *host, const char *port, const char *ssl_seed, const char *mam_seed,
                              const uint8_t *message, const size_t message_size, const uint8_t *private_key,
                              const size_t private_keySize, const char *device_id) {
  LE_DEBUG("endpoint: message length: %zu\nsend mam message:%s", message_size, message);
  int ret = send_mam_message(host, port, ssl_seed, mam_seed, message, message_size, private_key, device_id);
  return ret;
}

COMPONENT_INIT {}
