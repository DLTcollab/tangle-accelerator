/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "duplex_callback.h"

static status_t mqtt_request_handler(mosq_config_t *cfg, char *sub_topic, char *req) {
  status_t ret = SC_OK;
  // TODO: process MQTT requests here. Deserialize the request and process it against corresponding api_* functions

  // after finishing processing the request, set the response message with the following functions

  // 1. Set response publishing topic with the topic we got message and the Device ID (client ID) we got in the message/
  // gossip_channel_set()

  // 2. Set recv_message as publishing message
  // gossip_message_set(cfg, cfg->sub_config->recv_message);

  return ret;
}

static void publish_callback_duplex_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                                         const mosquitto_property *properties) {
  publish_callback_pub_func(mosq, obj, mid, reason_code, properties);
}

static void message_callback_duplex_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                                         const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  message_callback_sub_func(mosq, obj, message, properties);
  mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);

  // Process received requests
  mqtt_request_handler(cfg, message->topic, cfg->sub_config->recv_message);

  // After running `message_callback_duplex_func()`, the message will be sent by the `publish_loop()` in
  // `duplex_client_start()`
  // printf("listening: %s \n", message->topic);
  // printf("recv msg: %s \n", cfg->sub_config->recv_message);

  // The following one line is used for testing if this server work fine with requests with given topics.
  // Uncomment it if it is necessitated
  // gossip_message_set(cfg, cfg->sub_config->recv_message);
}

static void connect_callback_duplex_func(struct mosquitto *mosq, void *obj, int result, int flags,
                                         const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  if (cfg->general_config->client_type == client_pub) {
    connect_callback_pub_func(mosq, obj, result, flags, properties);
  } else if (cfg->general_config->client_type == client_sub) {
    connect_callback_sub_func(mosq, obj, result, flags, properties);
  }
}

static void disconnect_callback_duplex_func(struct mosquitto *mosq, void *obj, mosq_retcode_t ret,
                                            const mosquitto_property *properties) {
  // TODO we may necessitate doing something here after client is disconnected.
}

static void subscribe_callback_duplex_func(struct mosquitto *mosq, void *obj, int mid, int qos_count,
                                           const int *granted_qos) {
  subscribe_callback_sub_func(mosq, obj, mid, qos_count, granted_qos);
}

static void log_callback_duplex_func(struct mosquitto *mosq, void *obj, int level, const char *str) {
  printf("log: %s\n", str);
}

void duplex_callback_func_set(struct mosquitto *mosq) {
  mosquitto_log_callback_set(mosq, log_callback_duplex_func);
  mosquitto_subscribe_callback_set(mosq, subscribe_callback_duplex_func);
  mosquitto_connect_v5_callback_set(mosq, connect_callback_duplex_func);
  mosquitto_disconnect_v5_callback_set(mosq, disconnect_callback_duplex_func);
  mosquitto_publish_v5_callback_set(mosq, publish_callback_duplex_func);
  mosquitto_message_v5_callback_set(mosq, message_callback_duplex_func);
}
