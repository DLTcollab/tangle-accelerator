/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef PUB_UTILS_H
#define PUB_UTILS_H

#include "client_common.h"
#include "third_party/mosquitto/config.h"
#include "third_party/mosquitto/lib/mqtt_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/pub_utils.h
 */

void connect_callback_pub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties);
void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties);

/**
 * @brief Run `mosquitto_loop()` for publishing.
 *
 * @param[in] mosq `struct mosquitto` object
 * @param[in] cfg `mosq_config_t` object
 *
 * @return
 * - MOSQ_ERR_SUCCESS on success
 * - non-zero on error
 */
mosq_retcode_t publish_loop(struct mosquitto *mosq, mosq_config_t *cfg);

/**
 * @brief Check if any error happened after initialization.
 *
 * @param[in] cfg `mosq_config_t` object
 * @param[in] client_type client types
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t init_check_error(mosq_config_t *cfg, client_type_t client_type);

#ifdef __cplusplus
}
#endif

#endif  // PUB_UTILS_H
