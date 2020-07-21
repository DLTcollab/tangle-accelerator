/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef MQTT_SUB_UTILS_H_
#define MQTT_SUB_UTILS_H_

#include "mqtt_common.h"
#include "third_party/mosquitto/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/sub_utils.h
 * @brief MQTT subscriber utilities.
 */

/**
 * @brief Publish callback function of subscriber.
 *
 * when a message initiated with <mosquitto_publish> has been sent to the broker. This callback will be called both if
 * the message is sent successfully, or if the broker responded with an error, which will be reflected in the
 * reason_code parameter.
 */
void publish_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties);

/**
 * @brief Connect callback function of subscriber.
 *
 * This callback function is called when a message is received from the broker.
 */
void message_callback_sub_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                               const mosquitto_property *properties);

/**
 * @brief Connect callback function of subscriber.
 *
 * This callback function is called when the broker sends a CONNACK message in response to a connection owned by
 * subscriber.
 */
void connect_callback_sub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties);

/**
 * @brief Connect callback function of subscriber.
 *
 * This callback function is called when the broker responds to a subscription request.
 */
void subscribe_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);

#ifdef __cplusplus
}
#endif

#endif  // MQTT_SUB_UTILS_H_
