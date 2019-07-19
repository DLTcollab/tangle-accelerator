/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef DUPLEX_UTILS_H
#define DUPLEX_UTILS_H

#include "pub_utils.h"
#include "sub_utils.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/duplex_utils.h
 */

/**
 * @brief Initialize `mosq_config_t` object for a duplex client.
 *
 * @param[in] config_mosq pointer of pointer of `struct mosquitto` object
 * @param[in] config_cfg pointer of `mosq_config_t` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t duplex_config_init(struct mosquitto **config_mosq, mosq_config_t *config_cfg);

/**
 * @brief Set host and topics.
 *
 * The arguements `host` can be NULL value. But only one of value among `sub_topic` and `pub_topic` can be NULL.
 * Threfore this function can be used whenever we want to subscribe or publish to a new topic.
 *
 * @param[in] config_cfg pointer of `mosq_config_t` object
 * @param[in] host broker's IP in string
 * @param[in] sub_topic topic string for subscribing
 * @param[in] pub_topic topic string for publishing
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t gossip_channel_set(mosq_config_t *channel_cfg, char *host, char *sub_topic, char *pub_topic);

/**
 * @brief Set topics for TA to listen to.
 *
 * This function only set the topics that mqtt server will listen to (subscribe). It doesn't set the topics that mqtt
 * server is going to publish to.
 *
 * @param[in] config_cfg pointer of `mosq_config_t` object
 * @param[in] host broker's IP in string
 * @param[in] root_path root path of topics in the same system
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t gossip_api_channels_set(mosq_config_t *channel_cfg, char *host, char *root_path);

/**
 * @brief Set message that is going to be sent.
 *
 * @param[in] channel_cfg pointer of `mosq_config_t` object
 * @param[in] message message in string that going to be published
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t gossip_message_set(mosq_config_t *channel_cfg, char *message);

/**
 * @brief Start duplex client.
 *
 * This function start to run duplex client which is keep listening (subscribing) an assigned topic, and once
 * the duplex client receives message from publisher, the duplex client would start process this message (in TA we
 * process these requests as what we do in http server). After finishing processing the message (request), the duplex
 * client would publish the result to an assign topic. This function would call `mosq_client_connect()` to connect to
 * the broker and call `mosquitto_loop_forever()` to run infinite loop which allows us subscribing messages.
 *
 * @param[in] loop_mosq pointer of `struct mosquitto` object
 * @param[in] loop_cfg pointer of `mosq_config_t` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t duplex_client_start(struct mosquitto *loop_mosq, mosq_config_t *loop_cfg);

#ifdef __cplusplus
}
#endif

#endif  // DUPLEX_UTILS_H
