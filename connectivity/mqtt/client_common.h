/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CLIENT_CONFIG_H
#define CLIENT_CONFIG_H

#include <stdio.h>
#include "accelerator/errors.h"
#include "third_party/mosquitto/lib/mosquitto.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/mqtt/client_common.h
 */

#define ID_LEN 32
#define API_NUM 9

typedef enum client_type_s { client_pub, client_sub, client_duplex } client_type_t;

typedef enum mosq_err_t mosq_retcode_t;  // typedef the original enum

typedef struct mosq_general_config_s {
  char *id;
  int protocol_version;
  int keepalive;
  char *host;
  int port;
  int qos;
  int last_mid;
  bool retain;
  client_type_t client_type;
#ifdef WITH_SRV
  bool use_srv;
#endif
  unsigned int max_inflight;
  char *username;
  char *password;
  char *will_topic;
  char *will_payload;
  long will_payloadlen;
  int will_qos;
  bool will_retain;
  // char *bind_address;
  bool clean_session;
} mosq_general_config_t;

typedef struct mosq_pub_config_s {
  int mid_sent;
  char *message;
  long msglen;
  char *topic;
  bool first_publish;
  char *response_topic;
} mosq_pub_config_t;

typedef struct mosq_sub_config_s {
  char **topics;
  int topic_count;
  char *recv_message;
  bool no_retain;
  bool retained_only;
  bool remove_retained;
  char **unsub_topics;
  int unsub_topic_count;
} mosq_sub_config_t;

typedef struct mosq_property_config_s {
  mosquitto_property *connect_props;
  mosquitto_property *publish_props;
  mosquitto_property *subscribe_props;
  mosquitto_property *unsubscribe_props;
  mosquitto_property *disconnect_props;
  mosquitto_property *will_props;
} mosq_property_config_t;

#ifdef WITH_TLS
typedef struct mosq_tls_config_s {
  char *cafile;
  char *capath;
  char *certfile;
  char *keyfile;
  char *ciphers;
  bool insecure;
  char *tls_alpn;
  char *tls_version;
  char *tls_engine;
  char *tls_engine_kpass_sha1;
  char *keyform;
#ifdef FINAL_WITH_TLS_PSK
  char *psk;
  char *psk_identity;
#endif
} mosq_tls_config_t;
#endif

#ifdef WITH_SOCKS
typedef struct mosq_socks_config_s {
  char *socks5_host;
  int socks5_port;
  char *socks5_username;
  char *socks5_password;
} mosq_socks_config_t;
#endif

typedef struct mosq_config_s {
  mosq_general_config_t *general_config;
  mosq_pub_config_t *pub_config;
  mosq_sub_config_t *sub_config;
  mosq_property_config_t *property_config;
#ifdef WITH_TLS
  mosq_tls_config_t *tls_config;
#endif

#ifdef WITH_SOCKS
  mosq_socks_config_t *socks_config;
#endif
} mosq_config_t;

/**
 * @brief Initialize `mosq_config_t` object
 *
 * @param[in] cfg `mosq_config_t` object
 * @param[in] client_type client types
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
void init_mosq_config(mosq_config_t *cfg, client_type_t client_type);

/**
 * @brief Free `mosq_config_t` object.
 *
 * @param[in] cfg `mosq_config_t` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
void mosq_config_free(mosq_config_t *cfg);

/**
 * @brief Set `struct mosquitto` object options with values of `mosq_config_t` object.
 *
 * @param[in] mosq `struct mosquitto` object
 * @param[in] cfg `mosq_config_t` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t mosq_opts_set(struct mosquitto *mosq, mosq_config_t *cfg);

/**
 * @brief Generate MQTT client ID.
 *
 * @param[in] cfg `mosq_config_t` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t generate_client_id(mosq_config_t *cfg);

/**
 * @brief Let client connects to broker.
 *
 * @param[in] mosq `struct mosquitto` object
 * @param[in] cfg `mosq_config_t` object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t mosq_client_connect(struct mosquitto *mosq, mosq_config_t *cfg);

/**
 * @brief Add topic to `mosq_config_t` object
 *
 * @param[in] cfg `mosq_config_t` object
 * @param[in] client_type client types
 * @param[in] topic the topic that we are going to subscribe to or publish to.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cfg_add_topic(mosq_config_t *cfg, client_type_t client_type, char *topic);

#ifdef __cplusplus
}
#endif

#endif  // CLIENT_CONFIG_H
