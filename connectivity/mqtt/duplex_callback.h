/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef MQTT_DUPLEX_CALLBACK_H_
#define MQTT_DUPLEX_CALLBACK_H_

#include "accelerator/core/apis.h"
#include "common/model/transaction.h"
#include "duplex_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/duplex_callback.h
 * @brief Callback functions to handle MQTT requests
 */

/**
 * @breif Initialize ta_core
 *
 * @param[in] core `struct ta_core_t` object
 */
void ta_mqtt_init(ta_core_t *const core);

/**
 * @brief Interface for functions setting callback functions.
 *
 * @param[in] mosq `struct mosquitto` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t duplex_callback_func_set(struct mosquitto *mosq);

#ifdef __cplusplus
}
#endif

#endif  // MQTT_DUPLEX_CALLBACK_H_
