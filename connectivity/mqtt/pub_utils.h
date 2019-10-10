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
#undef uthash_free
#undef uthash_malloc
#include "third_party/mosquitto/config.h"
#include "third_party/mosquitto/lib/mqtt_protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/pub_utils.h
 */

/**
 * @brief Initialize logger
 */
void mqtt_pub_logger_init();

/**
 * @brief Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int mqtt_pub_logger_release();

/**
 * @brief Connect callback function of publisher.
 *
 * This callback function is called when the broker sends a CONNACK message in response to a connection owned by
 * publisher.
 */
void connect_callback_pub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties);

/**
 * @brief Publish callback function of publisher.
 *
 * when a message initiated with <mosquitto_publish> has been sent to the broker. This callback will be called both if
 * the message is sent successfully, or if the broker responded with an error, which will be reflected in the
 * reason_code parameter.
 */
void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties);

/**
 * @brief Run `mosquitto_loop()` for publishing.
 *
 * @param[in] mosq `struct mosquitto` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t publish_loop(struct mosquitto *mosq);

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
