/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "mqtt_common.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common/logger.h"

#define MQTT_COMMON_LOGGER "mqtt-common"
static logger_id_t logger_id;

void mqtt_common_logger_init() { logger_id = logger_helper_enable(MQTT_COMMON_LOGGER, LOGGER_DEBUG, true); }

int mqtt_common_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", MQTT_COMMON_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

#ifdef WITH_SOCKS
static int mosquitto__parse_socks_url(mosq_config_t *cfg, char *url);
#endif

status_t init_mosq_config(mosq_config_t *cfg, client_type_t client_type) {
  status_t ret = SC_OK;
  if (cfg == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  memset(cfg, 0, sizeof(mosq_config_t));

  cfg->general_config = (mosq_general_config_t *)malloc(sizeof(mosq_general_config_t));
  cfg->property_config = (mosq_property_config_t *)malloc(sizeof(mosq_property_config_t));
  if (cfg->general_config == NULL || cfg->property_config == NULL) {
    ta_log_error("%s\n", "SC_TA_OOM");
    ret = SC_MQTT_OOM;
    goto oom_err;
  }
  memset(cfg->general_config, 0, sizeof(mosq_general_config_t));
  memset(cfg->property_config, 0, sizeof(mosq_property_config_t));

#ifdef WITH_TLS
  cfg->tls_config = (mosq_tls_config_t *)malloc(sizeof(mosq_tls_config_t));
  if (cfg->tls_config == NULL) {
    ta_log_error("%s\n", "SC_TA_OOM");
    ret = SC_MQTT_OOM;
    goto oom_err;
  }
  memset(cfg->tls_config, 0, sizeof(mosq_tls_config_t));
#endif

#ifdef WITH_SOCKS
  cfg->socks_config = (mosq_socks_config_t *)malloc(sizeof(mosq_socks_config_t));
  if (cfg->socks_config == NULL) {
    ta_log_error("%s\n", "SC_TA_OOM");
    ret = SC_MQTT_OOM;
    goto oom_err;
  }
  memset(cfg->socks_config, 0, sizeof(mosq_socks_config_t));
#endif

  if ((client_type == client_pub) || (client_type == client_duplex)) {
    cfg->pub_config = (mosq_pub_config_t *)malloc(sizeof(mosq_pub_config_t));
    if (cfg->pub_config == NULL) {
      ta_log_error("%s\n", "SC_TA_OOM");
      ret = SC_MQTT_OOM;
      goto oom_err;
    }
    cfg->pub_config->first_publish = true;
    memset(cfg->pub_config, 0, sizeof(mosq_pub_config_t));
  }
  if ((client_type == client_sub) || (client_type == client_duplex)) {
    cfg->sub_config = (mosq_sub_config_t *)malloc(sizeof(mosq_sub_config_t));
    if (cfg->sub_config == NULL) {
      ta_log_error("%s\n", "SC_TA_OOM");
      ret = SC_MQTT_OOM;
      goto oom_err;
    }
    memset(cfg->sub_config, 0, sizeof(mosq_sub_config_t));
  }

  cfg->general_config->port = -1;
  cfg->general_config->qos = 1;
  cfg->general_config->max_inflight = 20;
  cfg->general_config->keepalive = 60;
  cfg->general_config->clean_session = true;
  cfg->general_config->protocol_version = MQTT_PROTOCOL_V311;
  cfg->general_config->client_type = client_type;

  return SC_OK;

oom_err:
  free(cfg->general_config);
  free(cfg->property_config);
  free(cfg->pub_config);
  free(cfg->sub_config);
#ifdef WITH_TLS
  free(cfg->tls_config);
#endif

#ifdef WITH_SOCKS
  free(cfg->socks_config);
#endif
  return ret;
}

status_t mosq_config_free(mosq_config_t *cfg) {
  if (cfg == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  if (cfg->general_config->client_type == client_pub || cfg->general_config->client_type == client_duplex) {
    free(cfg->pub_config->message);
    free(cfg->pub_config->topic);
    free(cfg->pub_config->response_topic);
  }
  if ((cfg->general_config->client_type == client_sub) || (cfg->general_config->client_type == client_duplex)) {
    if (cfg->sub_config->topics) {
      for (int i = 0; i < cfg->sub_config->topic_count; i++) {
        free(cfg->sub_config->topics[i]);
      }
      free(cfg->sub_config->topics);
    }
    if (cfg->sub_config->unsub_topics) {
      for (int i = 0; i < cfg->sub_config->unsub_topic_count; i++) {
        free(cfg->sub_config->unsub_topics[i]);
      }
      free(cfg->sub_config->unsub_topics);
    }
  }
  free(cfg->general_config->id);
  free(cfg->general_config->host);
  free(cfg->general_config->username);
  free(cfg->general_config->password);
  free(cfg->general_config->will_topic);
  free(cfg->general_config->will_payload);

#ifdef WITH_TLS
  free(cfg->tls_config->cafile);
  free(cfg->tls_config->capath);
  free(cfg->tls_config->certfile);
  free(cfg->tls_config->keyfile);
  free(cfg->tls_config->ciphers);
  free(cfg->tls_config->tls_alpn);
  free(cfg->tls_config->tls_version);
  free(cfg->tls_config->tls_engine);
  free(cfg->tls_config->tls_engine_kpass_sha1);
  free(cfg->tls_config->keyform);
#ifdef FINAL_WITH_TLS_PSK
  free(cfg->tls_config->psk);
  free(cfg->tls_config->psk_identity);
#endif
#endif

#ifdef WITH_SOCKS
  free(cfg->socks_config->socks5_host);
  free(cfg->socks_config->socks5_username);
  free(cfg->socks_config->socks5_password);
#endif
  mosquitto_property_free_all(&cfg->property_config->connect_props);
  mosquitto_property_free_all(&cfg->property_config->publish_props);
  mosquitto_property_free_all(&cfg->property_config->subscribe_props);
  mosquitto_property_free_all(&cfg->property_config->unsubscribe_props);
  mosquitto_property_free_all(&cfg->property_config->disconnect_props);
  mosquitto_property_free_all(&cfg->property_config->will_props);

  return SC_OK;
}

status_t cfg_add_topic(mosq_config_t *cfg, client_type_t client_type, char *topic) {
  if (cfg == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  if (mosquitto_validate_utf8(topic, strlen(topic))) {
    ta_log_error(":%s\n", "Error: Malformed UTF-8 in topic argument.");
    return SC_MQTT_TOPIC_SET;
  }

  if (mosquitto_pub_topic_check(topic) == MOSQ_ERR_INVAL) {
    ta_log_error(":%s\n", "Error: Invalid topic, does it contain '+' or '#'?");
    return SC_MQTT_TOPIC_SET;
  }

  if (client_type == client_pub || client_type == client_duplex) {
    cfg->pub_config->topic = strdup(topic);
  } else if (client_type == client_duplex) {
    cfg->pub_config->response_topic = strdup(topic);
  } else {
    cfg->sub_config->topic_count++;
    cfg->sub_config->topics = realloc(cfg->sub_config->topics, cfg->sub_config->topic_count * sizeof(char *));
    if (!cfg->sub_config->topics) {
      ta_log_error("%s\n", "SC_TA_OOM");
      return SC_TA_OOM;
    }
    cfg->sub_config->topics[cfg->sub_config->topic_count - 1] = strdup(topic);
  }
  return SC_OK;
}

status_t mosq_opts_set(struct mosquitto *mosq, mosq_config_t *cfg) {
  if (mosq == NULL || cfg == NULL) {
    ta_log_error("%s\n", "SC_MQTT_NULL");
    return SC_MQTT_NULL;
  }
#if defined(WITH_TLS) || defined(WITH_SOCKS)
  mosq_retcode_t ret;
#endif

  mosquitto_int_option(mosq, MOSQ_OPT_PROTOCOL_VERSION, cfg->general_config->protocol_version);

  if (cfg->general_config->will_topic &&
      mosquitto_will_set_v5(mosq, cfg->general_config->will_topic, cfg->general_config->will_payloadlen,
                            cfg->general_config->will_payload, cfg->general_config->will_qos,
                            cfg->general_config->will_retain, cfg->property_config->will_props)) {
    ta_log_error(":%s\n", "Error: Problem setting will.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
  cfg->property_config->will_props = NULL;

  if (cfg->general_config->username &&
      mosquitto_username_pw_set(mosq, cfg->general_config->username, cfg->general_config->password)) {
    ta_log_error(":%s\n", "Error: Problem setting username and password.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
#ifdef WITH_TLS
  if (cfg->tls_config->cafile || cfg->tls_config->capath) {
    ret = mosquitto_tls_set(mosq, cfg->tls_config->cafile, cfg->tls_config->capath, cfg->tls_config->certfile,
                            cfg->tls_config->keyfile, NULL);
    if (ret) {
      if (ret == MOSQ_ERR_INVAL) {
        ta_log_error(":%s\n", "Error: Problem setting TLS options: File not found.");
      } else {
        ta_log_error(":Error: Problem setting TLS options: %s.\n", mosquitto_strerror(ret));
      }
      mosquitto_lib_cleanup();
      return ret;
    }
  }
  if (cfg->tls_config->insecure && mosquitto_tls_insecure_set(mosq, true)) {
    ta_log_error(":%s\n", "Error: Problem setting TLS insecure option.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
  if (cfg->tls_config->tls_engine && mosquitto_string_option(mosq, MOSQ_OPT_TLS_ENGINE, cfg->tls_config->tls_engine)) {
    ta_log_error(":%s\n", "Error: Problem setting TLS engine.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
  if (cfg->tls_config->keyform && mosquitto_string_option(mosq, MOSQ_OPT_TLS_KEYFORM, cfg->tls_config->keyform)) {
    ta_log_error(":%s\n", "Error: Problem setting key form, it must be one of 'pem' or 'engine.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
  if (cfg->tls_config->tls_engine_kpass_sha1 &&
      mosquitto_string_option(mosq, MOSQ_OPT_TLS_ENGINE_KPASS_SHA1, cfg->tls_config->tls_engine_kpass_sha1)) {
    ta_log_error(":%s\n", "Error: Problem setting TLS engine key pass sha, is it a 40 character hex string?");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
  if (cfg->tls_config->tls_alpn && mosquitto_string_option(mosq, MOSQ_OPT_TLS_ALPN, cfg->tls_config->tls_alpn)) {
    ta_log_error(":%s\n", "Error: Problem setting TLS ALPN protocol.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
#ifdef FINAL_WITH_TLS_PSK
  if (cfg->tls_config->psk && mosquitto_tls_psk_set(mosq, cfg->tls_config->psk, cfg->tls_config->psk_identity, NULL)) {
    ta_log_error(":%s\n", "Error: Problem setting TLS-PSK options.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
#endif
  if ((cfg->tls_config->tls_version || cfg->tls_config->ciphers) &&
      mosquitto_tls_opts_set(mosq, 1, cfg->tls_config->tls_version, cfg->tls_config->ciphers)) {
    ta_log_error(":%s\n", "Error: Problem setting TLS options, check the options are valid.");
    mosquitto_lib_cleanup();
    return SC_MQTT_OPT_SET;
  }
#endif
  mosquitto_max_inflight_messages_set(mosq, cfg->general_config->max_inflight);
#ifdef WITH_SOCKS
  if (cfg->socks_config->socks5_host) {
    ret = mosquitto_socks5_set(mosq, cfg->socks_config->socks5_host, cfg->socks_config->socks5_port,
                               cfg->socks_config->socks5_username, cfg->socks_config->socks5_password);
    if (ret) {
      ta_log_error("%X\n", ret);
      mosquitto_lib_cleanup();
      return ret;
    }
  }
#endif
  return SC_OK;
}

status_t generate_client_id(mosq_config_t *cfg) {
  if (cfg == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  srand(time(NULL));
  char id_alphabet[] = "0123456789abcdefghijklmnopqrstuvwxyz";
  char id[ID_LEN];

  for (int i = 0; i < ID_LEN; i++) {
    id[i] = id_alphabet[rand() % 36];
  }
  cfg->general_config->id = (char *)malloc(sizeof(char) * ID_LEN);
  if (!cfg->general_config->id) {
    ta_log_error("%s\n", "SC_TA_OOM");
    mosquitto_lib_cleanup();
    return SC_MQTT_OOM;
  }

  strncpy(cfg->general_config->id, id, sizeof(char) * ID_LEN);

  return SC_OK;
}

status_t mosq_client_connect(struct mosquitto *mosq, mosq_config_t *cfg) {
  if (mosq == NULL || cfg == NULL) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  mosq_retcode_t ret;
  char *err;
  int port;

  if (cfg->general_config->port < 0) {
#ifdef WITH_TLS
    if (cfg->tls_config->cafile || cfg->tls_config->capath
#ifdef FINAL_WITH_TLS_PSK
        || cfg->tls_config->psk
#endif
    ) {
      port = 8883;
    } else
#endif
    {
      port = 1883;
    }
  } else {
    port = cfg->general_config->port;
  }

#ifdef WITH_SRV
  if (cfg->use_srv) {
    ret = mosquitto_connect_srv(mosq, cfg->general_config->host, cfg->general_config->keepalive,
                                cfg->general_config->bind_address);
  } else {
    ret = mosquitto_connect_bind_v5(mosq, cfg->general_config->host, port, cfg->general_config->keepalive,
                                    cfg->general_config->bind_address, cfg->property_config->connect_props);
  }
#else
  ret = mosquitto_connect_bind_v5(mosq, cfg->general_config->host, port, cfg->general_config->keepalive, NULL,
                                  cfg->property_config->connect_props);
#endif
  if (ret) {
    {
      if (ret == MOSQ_ERR_ERRNO) {
        err = strerror(errno);
        ta_log_error(":%s\n", err);
      } else {
        ta_log_error(":%s\n", mosquitto_strerror(ret));
      }
    }
    mosquitto_lib_cleanup();
    return SC_CLIENT_CONNECT;
  }
  return SC_OK;
}

#ifdef WITH_SOCKS
/* Convert %25 -> %, %3a, %3A -> :, %40 -> @ */
static int mosquitto__urldecode(char *str) {
  int i, j;
  int len;
  if (!str) return 0;

  if (!strethr(str, '%')) return 0;

  len = strlen(str);
  for (i = 0; i < len; i++) {
    if (str[i] == '%') {
      if (i + 2 >= len) {
        return EXIT_FAILURE;
      }
      if (str[i + 1] == '2' && str[i + 2] == '5') {
        str[i] = '%';
        len -= 2;
        for (j = i + 1; j < len; j++) {
          str[j] = str[j + 2];
        }
        str[j] = '\0';
      } else if (str[i + 1] == '3' && (str[i + 2] == 'A' || str[i + 2] == 'a')) {
        str[i] = ':';
        len -= 2;
        for (j = i + 1; j < len; j++) {
          str[j] = str[j + 2];
        }
        str[j] = '\0';
      } else if (str[i + 1] == '4' && str[i + 2] == '0') {
        str[i] = ':';
        len -= 2;
        for (j = i + 1; j < len; j++) {
          str[j] = str[j + 2];
        }
        str[j] = '\0';
      } else {
        return EXIT_FAILURE;
      }
    }
  }
  return 0;
}

static int mosquitto__parse_socks_url(mosq_config_t *cfg, char *url) {
  char *str;
  size_t i;
  char *username = NULL, *password = NULL, *host = NULL, *port = NULL;
  char *username_or_host = NULL;
  size_t start;
  size_t len;
  bool have_auth = false;
  int port_int;

  if (!strncmp(url, "socks5h://", strlen("socks5h://"))) {
    str = url + strlen("socks5h://");
  } else {
    fprintf(stderr, "Error: Unsupported proxy protocol: %s\n", url);
    return EXIT_FAILURE;
  }

  // socks5h://username:password@host:1883
  // socks5h://username:password@host
  // socks5h://username@host:1883
  // socks5h://username@host
  // socks5h://host:1883
  // socks5h://host

  start = 0;
  int str_len = strlen(str);
  for (i = 0; i < str_len; i++) {
    if (str[i] == ':') {
      if (i == start) {
        goto cleanup;
      }
      if (have_auth) {
        /* Have already seen a @ , so this must be of form
         * socks5h://username[:password]@host:port */
        if (host) {
          /* Already seen a host, must be malformed. */
          goto cleanup;
        }
        len = i - start;
        host = malloc(len + 1);
        if (!host) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(host, &(str[start]), len);
        host[len] = '\0';
        start = i + 1;
      } else if (!username_or_host) {
        /* Haven't seen a @ before, so must be of form
         * socks5h://host:port or
         * socks5h://username:password@host[:port] */
        len = i - start;
        username_or_host = malloc(len + 1);
        if (!username_or_host) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(username_or_host, &(str[start]), len);
        username_or_host[len] = '\0';
        start = i + 1;
      }
    } else if (str[i] == '@') {
      if (i == start) {
        goto cleanup;
      }
      have_auth = true;
      if (username_or_host) {
        /* Must be of form socks5h://username:password@... */
        username = username_or_host;
        username_or_host = NULL;

        len = i - start;
        password = malloc(len + 1);
        if (!password) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(password, &(str[start]), len);
        password[len] = '\0';
        start = i + 1;
      } else {
        /* Haven't seen a : yet, so must be of form
         * socks5h://username@... */
        if (username) {
          /* Already got a username, must be malformed. */
          goto cleanup;
        }
        len = i - start;
        username = malloc(len + 1);
        if (!username) {
          fprintf(stderr, "Error: Out of memory.\n");
          goto cleanup;
        }
        memcpy(username, &(str[start]), len);
        username[len] = '\0';
        start = i + 1;
      }
    }
  }

  /* Deal with remainder */
  if (i > start) {
    len = i - start;
    if (host) {
      /* Have already seen a @ , so this must be of form
       * socks5h://username[:password]@host:port */
      port = malloc(len + 1);
      if (!port) {
        fprintf(stderr, "Error: Out of memory.\n");
        goto cleanup;
      }
      memcpy(port, &(str[start]), len);
      port[len] = '\0';
    } else if (username_or_host) {
      /* Haven't seen a @ before, so must be of form
       * socks5h://host:port */
      host = username_or_host;
      username_or_host = NULL;
      port = malloc(len + 1);
      if (!port) {
        fprintf(stderr, "Error: Out of memory.\n");
        goto cleanup;
      }
      memcpy(port, &(str[start]), len);
      port[len] = '\0';
    } else {
      host = malloc(len + 1);
      if (!host) {
        fprintf(stderr, "Error: Out of memory.\n");
        goto cleanup;
      }
      memcpy(host, &(str[start]), len);
      host[len] = '\0';
    }
  }

  if (!host) {
    fprintf(stderr, "Error: Invalid proxy.\n");
    goto cleanup;
  }

  if (mosquitto__urldecode(username)) {
    goto cleanup;
  }
  if (mosquitto__urldecode(password)) {
    goto cleanup;
  }
  if (port) {
    port_int = atoi(port);
    if (port_int < 1 || port_int > 65535) {
      fprintf(stderr, "Error: Invalid proxy port %d\n", port_int);
      goto cleanup;
    }
    free(port);
  } else {
    port_int = 1080;
  }

  cfg->socks_config->socks5_username = username;
  cfg->socks_config->socks5_password = password;
  cfg->socks_config->socks5_host = host;
  cfg->socks_config->socks5_port = port_int;

  return 0;
cleanup:
  free(username_or_host);
  free(username);
  free(password);
  free(host);
  free(port);
  return EXIT_FAILURE;
}
#endif
