/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "pub_utils.h"
#include <string.h>
#include <time.h>

mosq_retcode_t publish_message(struct mosquitto *mosq, mosq_config_t *cfg, int *mid, const char *topic, int payloadlen,
                               void *payload, int qos, bool retain) {
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
            fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
            break;
          case MOSQ_ERR_NOMEM:
            fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
            break;
          case MOSQ_ERR_NO_CONN:
            fprintf(stderr, "Error: Client not connected when trying to publish.\n");
            break;
          case MOSQ_ERR_PROTOCOL:
            fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
            break;
          case MOSQ_ERR_PAYLOAD_SIZE:
            fprintf(stderr, "Error: Message payload is too large.\n");
            break;
          case MOSQ_ERR_QOS_NOT_SUPPORTED:
            fprintf(stderr, "Error: Message QoS not supported on broker, try a lower QoS.\n");
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
        fprintf(stderr, "%s\n", mosquitto_reason_string(result));
      } else {
        fprintf(stderr, "%s\n", mosquitto_connack_string(result));
      }
    }
  }
}

void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(properties);

  if (reason_code > 127) {
    fprintf(stderr, "Warning: Publish %d failed: %s.\n", mid, mosquitto_reason_string(reason_code));
  }

  mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
}

mosq_retcode_t publish_loop(struct mosquitto *mosq, mosq_config_t *cfg) {
  mosq_retcode_t ret = MOSQ_ERR_SUCCESS;

  do {
    ret = mosquitto_loop(mosq, 0, 1);
  } while (ret == MOSQ_ERR_SUCCESS);
  return MOSQ_ERR_SUCCESS;
}

status_t init_check_error(mosq_config_t *cfg, client_type_t client_type) {
  status_t ret = SC_OK;

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    fprintf(stderr, "Warning: Not using password since username not set.\n");
  }

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    fprintf(stderr, "Warning: Not using password since username not set.\n");
  }
#ifdef WITH_TLS
  if ((cfg->tls_config->certfile && !cfg->tls_config->keyfile) ||
      (cfg->tls_config->keyfile && !cfg->tls_config->certfile)) {
    fprintf(stderr, "Error: Both certfile and keyfile must be provided if one of them is set.\n");
    return SC_MQTT_INIT;
  }
  if ((cfg->tls_config->keyform && !cfg->tls_config->keyfile)) {
    fprintf(stderr, "Error: If keyform is set, keyfile must be also specified.\n");
    return SC_MQTT_INIT;
  }
  if ((cfg->tls_config->tls_engine_kpass_sha1 && (!cfg->tls_config->keyform || !cfg->tls_config->tls_engine))) {
    fprintf(stderr, "Error: when using tls-engine-kpass-sha1, both tls-engine and keyform must also be provided.\n");
    return SC_MQTT_INIT;
  }
#endif
#ifdef FINAL_WITH_TLS_PSK
  if ((cfg->tls_config->cafile || cfg->tls_config->capath) && cfg->tls_config->psk) {
    fprintf(stderr, "Error: Only one of --psk or --cafile/--capath may be used at once.\n");
    return SC_MQTT_INIT;
  }
  if (cfg->tls_config->psk && !cfg->tls_config->psk_identity) {
    fprintf(stderr, "Error: --psk-identity required if --psk used.\n");
    return SC_MQTT_INIT;
  }
#endif

  if (cfg->general_config->clean_session == false && !cfg->general_config->id) {
    fprintf(stderr, "Error: You must provide a client id if you are using the -c option.\n");
    return SC_MQTT_INIT;
  }

  if (client_type == client_sub) {
    if (cfg->sub_config->topic_count == 0) {
      fprintf(stderr, "Error: You must specify a topic to subscribe to.\n");
      return SC_MQTT_INIT;
    }
  }

  if (!cfg->general_config->host) {
    cfg->general_config->host = strdup("localhost");
    if (!cfg->general_config->host) {
      fprintf(stderr, "Error: Out of memory.\n");
      return SC_MQTT_INIT;
    }
  }

  ret = mosquitto_property_check_all(CMD_CONNECT, cfg->property_config->connect_props);
  if (ret) {
    fprintf(stderr, "Error in CONNECT properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_PUBLISH, cfg->property_config->publish_props);
  if (ret) {
    fprintf(stderr, "Error in PUBLISH properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_SUBSCRIBE, cfg->property_config->subscribe_props);
  if (ret) {
    fprintf(stderr, "Error in SUBSCRIBE properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_UNSUBSCRIBE, cfg->property_config->unsubscribe_props);
  if (ret) {
    fprintf(stderr, "Error in UNSUBSCRIBE properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_DISCONNECT, cfg->property_config->disconnect_props);
  if (ret) {
    fprintf(stderr, "Error in DISCONNECT properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }
  ret = mosquitto_property_check_all(CMD_WILL, cfg->property_config->will_props);
  if (ret) {
    fprintf(stderr, "Error in Will properties: %s\n", mosquitto_strerror(ret));
    return SC_MQTT_INIT;
  }

  return MOSQ_ERR_SUCCESS;
}
