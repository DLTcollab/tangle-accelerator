/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SUB_UTILS_H
#define SUB_UTILS_H

#include "client_common.h"
#include "third_party/mosquitto/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/sub_utils.h
 */

void publish_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties);
void message_callback_sub_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                               const mosquitto_property *properties);
void connect_callback_sub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties);
void subscribe_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);

#ifdef __cplusplus
}
#endif

#endif  // SUB_UTILS_H
