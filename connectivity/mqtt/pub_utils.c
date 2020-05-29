/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "pub_utils.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common/logger.h"

#define MQTT_PUB_LOGGER "pub_utils"
static logger_id_t logger_id;

void mqtt_pub_logger_init() { logger_id = logger_helper_enable(MQTT_PUB_LOGGER, LOGGER_DEBUG, true); }

int mqtt_pub_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", MQTT_PUB_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

mosq_retcode_t publish_message(struct mosquitto *mosq, mosq_config_t *cfg, int *mid, const char *topic, int payloadlen,
                               void *payload, int qos, bool retain) {
  if (mosq == NULL || cfg == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
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
            ta_log_error(":%s.\n", "Error: Invalid input. Does your topic contain '+' or '#'?");
            break;
          case MOSQ_ERR_NOMEM:
            ta_log_error(":%s.\n", "Error: Out of memory when trying to publish message.");
            break;
          case MOSQ_ERR_NO_CONN:
            ta_log_error(":%s.\n", "Error: Client not connected when trying to publish.");
            break;
          case MOSQ_ERR_PROTOCOL:
            ta_log_error(":%s.\n", "Error: Protocol error when communicating with broker.");
            break;
          case MOSQ_ERR_PAYLOAD_SIZE:
            ta_log_error(":%s.\n", "Error: Message payload is too large.");
            break;
          case MOSQ_ERR_QOS_NOT_SUPPORTED:
            ta_log_error(":%s.\n", "Error: Message QoS not supported on broker, try a lower QoS.");
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
        ta_log_error(":%s.\n", mosquitto_reason_string(result));
      } else {
        ta_log_error(":%s.\n", mosquitto_connack_string(result));
      }
    }
  }
}

void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(properties);

  if (reason_code > 127) {
    ta_log_error("Warning: Publish %d failed: %s.\n", mid, mosquitto_reason_string(reason_code));
  }

  mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
}

status_t publish_loop(struct mosquitto *mosq) {
  if (mosq == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }
  mosq_retcode_t ret = MOSQ_ERR_SUCCESS;

  do {
    ret = mosquitto_loop(mosq, 0, 1);
  } while (ret == MOSQ_ERR_SUCCESS);
  return SC_OK;
}

status_t init_check_error(mosq_config_t *cfg, client_type_t client_type) {
  if (cfg == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }
  status_t ret = SC_OK;

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    ta_log_error(":%s\n", "Error: Will payload given, but no will topic given.");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    ta_log_error("Error: Will retain given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    ta_log_error("Warning: Not using password since username not set.\n");
  }

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    ta_log_error("Error: Will payload given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    ta_log_error("Error: Will retain given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    ta_log_error("Warning: Not using password since username not set.\n");
  }
#ifdef WITH_TLS
  if ((cfg->tls_config->certfile && !cfg->tls_config->keyfile) ||
      (cfg->tls_config->keyfile && !cfg->tls_config->certfile)) {
    ta_log_error("Error: Both certfile and keyfile must be provided if one of them is set.\n");
    return SC_MQTT_INIT;
  }
  if ((cfg->tls_config->keyform && !cfg->tls_config->keyfile)) {
    ta_log_error("Error: If keyform is set, keyfile must be also specified\n");
    return SC_MQTT_INIT;
  }
  if ((cfg->tls_config->tls_engine_kpass_sha1 && (!cfg->tls_config->keyform || !cfg->tls_config->tls_engine))) {
    ta_log_error("Error: when using tls-engine-kpass-sha1, both tls-engine and keyform must also be provided\n");
    return SC_MQTT_INIT;
  }
#endif
#ifdef FINAL_WITH_TLS_PSK
  if ((cfg->tls_config->cafile || cfg->tls_config->capath) && cfg->tls_config->psk) {
    ta_log_error("Error: Only one of --psk or --cafile/--capath may be used at once.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->tls_config->psk && !cfg->tls_config->psk_identity) {
    ta_log_error("Error: --psk-identity required if --psk used\n");
    return SC_MQTT_INIT;
  }
#endif

  if (cfg->general_config->clean_session == false && !cfg->general_config->id) {
    ta_log_error("Error: You must provide a client id\n");
    return SC_MQTT_INIT;
  }

  if (client_type == client_sub) {
    if (cfg->sub_config->topic_count == 0) {
      ta_log_error("Error: You must specify a topic to subscribe to\n");
      return SC_MQTT_INIT;
    }
  }

  if (!cfg->general_config->host) {
    cfg->general_config->host = strdup("localhost");
    if (!cfg->general_config->host) {
      ta_log_error("%s\n", ta_error_to_string(SC_OOM));
      return SC_OOM;
    }
  }

  ret = mosquitto_property_check_all(CMD_CONNECT, cfg->property_config->connect_props);
  if (ret) {
    ta_log_error("Error in CONNECT properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_PUBLISH, cfg->property_config->publish_props);
  if (ret) {
    ta_log_error("Error in PUBLISH properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_SUBSCRIBE, cfg->property_config->subscribe_props);
  if (ret) {
    ta_log_error("Error in SUBSCRIBE properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_UNSUBSCRIBE, cfg->property_config->unsubscribe_props);
  if (ret) {
    ta_log_error("Error in UNSUBSCRIBE properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_DISCONNECT, cfg->property_config->disconnect_props);
  if (ret) {
    ta_log_error("Error in DISCONNECT properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_WILL, cfg->property_config->will_props);
  if (ret) {
    ta_log_error("Error in Will properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }

  return MOSQ_ERR_SUCCESS;
}
