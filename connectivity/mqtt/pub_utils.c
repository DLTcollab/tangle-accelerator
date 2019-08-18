/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "pub_utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils/logger_helper.h"

#define MQTT_PUB_LOGGER "mqtt-pub"
static logger_id_t mqtt_pub_logger_id;

void mqtt_pub_logger_init() { mqtt_pub_logger_id = logger_helper_enable(MQTT_PUB_LOGGER, LOGGER_DEBUG, true); }

int mqtt_pub_logger_release() {
  logger_helper_release(mqtt_pub_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(mqtt_pub_logger_id, "[%s:%s] Destroying logger failed %s.\n", __func__, __LINE__, MQTT_PUB_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

mosq_retcode_t publish_message(struct mosquitto *mosq, mosq_config_t *cfg, int *mid, const char *topic, int payloadlen,
                               void *payload, int qos, bool retain) {
  if (mosq == NULL || cfg == NULL) {
    log_error(mqtt_pub_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  if (cfg->general_config->protocol_version == MQTT_PROTOCOL_V5 && cfg->pub_config->first_publish == false) {
    return mosquitto_publish_v5(mosq, mid, NULL, payloadlen, payload, qos, retain, cfg->property_config->publish_props);
  } else {
    cfg->pub_config->first_publish = false;
    return mosquitto_publish_v5(mosq, mid, topic, payloadlen, payload, qos, retain,
                                cfg->property_config->publish_props);
  }
}

void connect_callback_pub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties) {
  mosq_retcode_t ret;
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(flags);
  UNUSED(properties);

  if (!result) {
    ret = publish_message(mosq, cfg, &cfg->pub_config->mid_sent, cfg->pub_config->topic, cfg->pub_config->msglen,
                          cfg->pub_config->message, cfg->general_config->qos, cfg->general_config->retain);
    if (ret) {
      {
        switch (ret) {
          case MOSQ_ERR_INVAL:
            log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__,
                      "Error: Invalid input. Does your topic contain '+' or '#'?");
            break;
          case MOSQ_ERR_NOMEM:
            log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__,
                      "Error: Out of memory when trying to publish message.");
            break;
          case MOSQ_ERR_NO_CONN:
            log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__,
                      "Error: Client not connected when trying to publish.");
            break;
          case MOSQ_ERR_PROTOCOL:
            log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__,
                      "Error: Protocol error when communicating with broker.");
            break;
          case MOSQ_ERR_PAYLOAD_SIZE:
            log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__, "Error: Message payload is too large.");
            break;
          case MOSQ_ERR_QOS_NOT_SUPPORTED:
            log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__,
                      "Error: Message QoS not supported on broker, try a lower QoS.");
            break;
          default:
            break;
        }
      }
      mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
    }
  } else {
    if (result) {
      if (cfg->general_config->protocol_version == MQTT_PROTOCOL_V5) {
        log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__, mosquitto_reason_string(result));
      } else {
        log_error(mqtt_pub_logger_id, "[%s:%d]:%s.\n", __func__, __LINE__, mosquitto_connack_string(result));
      }
    }
  }
}

void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(properties);

  if (reason_code > 127) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Warning: Publish %d failed: %s.\n", __func__, __LINE__, mid,
              mosquitto_reason_string(reason_code));
  }

  mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
}

status_t publish_loop(struct mosquitto *mosq) {
  if (mosq == NULL) {
    log_error(mqtt_pub_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MQTT_NULL");
    return SC_MQTT_NULL;
  }
  mosq_retcode_t ret = MOSQ_ERR_SUCCESS;

  do {
    ret = mosquitto_loop(mosq, 0, 1);
  } while (ret == MOSQ_ERR_SUCCESS);
  return SC_OK;
}

status_t init_check_error(mosq_config_t *cfg, client_type_t client_type) {
  if (cfg == NULL) {
    log_error(mqtt_pub_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }
  status_t ret = SC_OK;

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    log_error(mqtt_pub_logger_id, "[%s:%d]:%s\n", __func__, __LINE__,
              "Error: Will payload given, but no will topic given.");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: Will retain given, but no will topic given.\n", __func__, __LINE__);
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Warning: Not using password since username not set.\n", __func__, __LINE__);
  }

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: Will payload given, but no will topic given.\n", __func__, __LINE__);
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: Will retain given, but no will topic given.\n", __func__, __LINE__);
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Warning: Not using password since username not set.\n", __func__, __LINE__);
  }
#ifdef WITH_TLS
  if ((cfg->tls_config->certfile && !cfg->tls_config->keyfile) ||
      (cfg->tls_config->keyfile && !cfg->tls_config->certfile)) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: Both certfile and keyfile must be provided if one of them is set.\n",
              __func__, __LINE__);
    return SC_MQTT_INIT;
  }
  if ((cfg->tls_config->keyform && !cfg->tls_config->keyfile)) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: If keyform is set, keyfile must be also specified\n", __func__,
              __LINE__);
    return SC_MQTT_INIT;
  }
  if ((cfg->tls_config->tls_engine_kpass_sha1 && (!cfg->tls_config->keyform || !cfg->tls_config->tls_engine))) {
    log_error(mqtt_pub_logger_id,
              "[%s:%d]Error: when using tls-engine-kpass-sha1, both tls-engine and keyform must also be provided\n",
              __func__, __LINE__);
    return SC_MQTT_INIT;
  }
#endif
#ifdef FINAL_WITH_TLS_PSK
  if ((cfg->tls_config->cafile || cfg->tls_config->capath) && cfg->tls_config->psk) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: Only one of --psk or --cafile/--capath may be used at once.\n",
              __func__, __LINE__);
    return SC_MQTT_INIT;
  }
  if (cfg->tls_config->psk && !cfg->tls_config->psk_identity) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: --psk-identity required if --psk used\n", __func__, __LINE__);
    return SC_MQTT_INIT;
  }
#endif

  if (cfg->general_config->clean_session == false && !cfg->general_config->id) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error: You must provide a client id\n", __func__, __LINE__);
    return SC_MQTT_INIT;
  }

  if (client_type == client_sub) {
    if (cfg->sub_config->topic_count == 0) {
      log_error(mqtt_pub_logger_id, "[%s:%d]Error: You must specify a topic to subscribe to\n", __func__, __LINE__);
      return SC_MQTT_INIT;
    }
  }

  if (!cfg->general_config->host) {
    cfg->general_config->host = strdup("localhost");
    if (!cfg->general_config->host) {
      log_error(mqtt_pub_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_MQTT_OOM");
      return SC_MQTT_OOM;
    }
  }

  ret = mosquitto_property_check_all(CMD_CONNECT, cfg->property_config->connect_props);
  if (ret) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error in CONNECT properties: %s\n", __func__, __LINE__,
              mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_PUBLISH, cfg->property_config->publish_props);
  if (ret) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error in PUBLISH properties: %s\n", __func__, __LINE__,
              mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_SUBSCRIBE, cfg->property_config->subscribe_props);
  if (ret) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error in SUBSCRIBE properties: %s\n", __func__, __LINE__,
              mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_UNSUBSCRIBE, cfg->property_config->unsubscribe_props);
  if (ret) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error in UNSUBSCRIBE properties: %s\n", __func__, __LINE__,
              mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_DISCONNECT, cfg->property_config->disconnect_props);
  if (ret) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error in DISCONNECT properties: %s\n", __func__, __LINE__,
              mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_WILL, cfg->property_config->will_props);
  if (ret) {
    log_error(mqtt_pub_logger_id, "[%s:%d]Error in Will properties: %s\n", __func__, __LINE__, mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }

  return MOSQ_ERR_SUCCESS;
}
