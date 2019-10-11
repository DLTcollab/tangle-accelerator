/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "sub_utils.h"
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include "utils/logger_helper.h"

#define MQTT_SUB_LOGGER "mqtt-sub"
static logger_id_t logger_id;

void mqtt_sub_logger_init() { logger_id = logger_helper_enable(MQTT_SUB_LOGGER, LOGGER_DEBUG, true); }

int mqtt_sub_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_critical("Destroying logger failed %s.\n", MQTT_SUB_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

static status_t dump_message(mosq_config_t *cfg, const struct mosquitto_message *message) {
  if (cfg == NULL || message == NULL || message->payload == NULL || message->payloadlen == 0) {
    ta_log_error("%s\n", "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  cfg->sub_config->recv_message = strdup(message->payload);
  return SC_OK;
}

void publish_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(reason_code);
  UNUSED(properties);

  if ((mid == cfg->general_config->last_mid || cfg->general_config->last_mid == 0)) {
    mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
  }
}

void message_callback_sub_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(properties);

  if (cfg->sub_config->remove_retained && message->retain) {
    mosquitto_publish(mosq, &cfg->general_config->last_mid, message->topic, 0, NULL, 1, true);
  }

  if (cfg->sub_config->retained_only && !message->retain) {
    if (cfg->general_config->last_mid == 0) {
      mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
    }
    return;
  }

  if (message->retain && cfg->sub_config->no_retain) return;

  // Copy the message into `cfg` to allow other functions use the message
  dump_message(cfg, message);

  // Uncomment the following code would cause: once we received a message, then disconnect the connection.
  // mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
}

void connect_callback_sub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;

  UNUSED(flags);
  UNUSED(properties);

  if (!result) {
    mosquitto_subscribe_multiple(mosq, NULL, cfg->sub_config->topic_count, cfg->sub_config->topics,
                                 cfg->general_config->qos, 0, cfg->property_config->subscribe_props);

    for (int i = 0; i < cfg->sub_config->unsub_topic_count; i++) {
      mosquitto_unsubscribe_v5(mosq, NULL, cfg->sub_config->unsub_topics[i], cfg->property_config->unsubscribe_props);
    }
  } else {
    if (result) {
      if (cfg->general_config->protocol_version == MQTT_PROTOCOL_V5) {
        ta_log_error(":%s\n", mosquitto_reason_string(result));
      } else {
        ta_log_error(":%s\n", mosquitto_connack_string(result));
      }
    }
    mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
  }
}

void subscribe_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
  UNUSED(mosq);
  UNUSED(obj);
  char *qos_str = (char *)calloc(qos_count, 3);
  char qos_digit[4];
  for (int i = 1; i < qos_count; i++) {
    snprintf(qos_digit, 4, ", %d", granted_qos[i]);
    strcat(qos_str, qos_digit);
  }
  log_info(logger_id, "Subscribed (mid: %d): %d%s\n", mid, granted_qos[0], qos_str);

  free(qos_str);
}
