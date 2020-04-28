/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "duplex_callback.h"
#include <stdlib.h>
#include <string.h>
#include "connectivity/common.h"

#define MQTT_CALLBACK_LOGGER "duplex_callback"
static logger_id_t logger_id;

extern ta_core_t ta_core;

void mqtt_callback_logger_init() { logger_id = logger_helper_enable(MQTT_CALLBACK_LOGGER, LOGGER_DEBUG, true); }

int mqtt_callback_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", MQTT_CALLBACK_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t check_valid_tag(char *tag) {
  size_t len = strnlen(tag, NUM_TRYTES_TAG + 1);
  if (len < 1 || len > NUM_TRYTES_TAG) {
    ta_log_error("%s\n", ta_error_to_string(SC_MQTT_INVALID_TAG));
    return SC_MQTT_INVALID_TAG;
  }

  for (size_t i = 0; i < len; ++i) {
    if ((tag[i] > 'Z' || tag[i] < 'A') && tag[i] != '9') {
      ta_log_error("%s\n", ta_error_to_string(SC_MQTT_INVALID_TAG));
      return SC_MQTT_INVALID_TAG;
    }
  }

  return SC_OK;
}

static status_t mqtt_request_handler(mosq_config_t *cfg, char *subscribe_topic, char *req) {
  if (cfg == NULL || subscribe_topic == NULL || req == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_MQTT_NULL));
    return SC_MQTT_NULL;
  }

  status_t ret = SC_OK;
  char *json_result = NULL;
  char device_id[ID_LEN] = {0};

  // get the Device ID.
  ret = mqtt_device_id_deserialize(req, device_id);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  char *api_sub_topic = subscribe_topic + strlen(ta_core.ta_conf.mqtt_topic_root);
  if (api_path_matcher(api_sub_topic, "/address") == SC_OK)
    ret = api_generate_address(&ta_core.iota_conf, &ta_core.iota_service, &json_result);
  else if (api_path_matcher(api_sub_topic, "/tag/hashes") == SC_OK) {
    char tag[NUM_TRYTES_TAG + 1] = {0};
    mqtt_tag_req_deserialize(req, tag);
    if (check_valid_tag(tag) == SC_OK) {
      ret = api_find_transactions_by_tag(&ta_core.iota_service, tag, &json_result);
    } else {
      ret = SC_HTTP_URL_NOT_MATCH;
    }
  } else if (api_path_matcher(api_sub_topic, "/tag/object") == SC_OK) {
    char tag[NUM_TRYTES_TAG + 1] = {0};
    mqtt_tag_req_deserialize(req, tag);
    if (check_valid_tag(tag) == SC_OK) {
      ret = api_find_transactions_obj_by_tag(&ta_core.iota_service, tag, &json_result);
    } else {
      ret = SC_HTTP_URL_NOT_MATCH;
    }
  } else if (api_path_matcher(api_sub_topic, "/transaction") == SC_OK) {
    char hash[NUM_TRYTES_HASH + 1];
    mqtt_transaction_hash_req_deserialize(req, hash);
    ret = api_find_transaction_object_single(&ta_core.iota_service, hash, &json_result);
  } else if (api_path_matcher(api_sub_topic, "/transaction/object") == SC_OK) {
    ret = api_find_transaction_objects(&ta_core.iota_service, req, &json_result);
  } else if (api_path_matcher(api_sub_topic, "/transaction/send") == SC_OK) {
    ret = api_send_transfer(&ta_core, req, &json_result);
  } else if (api_path_matcher(api_sub_topic, "/tips/all") == SC_OK)
    ret = api_get_tips(&ta_core.iota_service, &json_result);
  else if (api_path_matcher(api_sub_topic, "/tips/pair") == SC_OK)
    ret = api_get_tips_pair(&ta_core.iota_conf, &ta_core.iota_service, &json_result);
  else if (api_path_matcher(api_sub_topic, "/tryte") == SC_OK)
    ret = api_send_trytes(&ta_core.ta_conf, &ta_core.iota_conf, &ta_core.iota_service, req, &json_result);
  else {
    cJSON *json_obj = cJSON_CreateObject();
    cJSON_AddStringToObject(json_obj, "message", api_sub_topic);
    json_result = cJSON_PrintUnformatted(json_obj);
    cJSON_Delete(json_obj);
  }
  set_response_content(ret, &json_result);

  // Log results
  printf("\"%s\" %s\n", api_sub_topic, ta_error_to_string(ret));

  // Set response publishing topic with the topic we got message and the Device ID (client ID) we got in the message
  int res_topic_len = strlen(subscribe_topic) + 1 + ID_LEN + 1;
  char *res_topic = (char *)malloc(res_topic_len);
  snprintf(res_topic, res_topic_len, "%s/%s", subscribe_topic, device_id);
  ret = gossip_channel_set(cfg, NULL, NULL, res_topic);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
    goto done;
  }

  // Set recv_message as publishing message
  ret = gossip_message_set(cfg, json_result);
  if (ret != SC_OK) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

done:
  free(json_result);
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
  UNUSED(mosq);
  UNUSED(obj);
  UNUSED(ret);
  UNUSED(properties);
  // TODO we may necessitate doing something here after client is disconnected.
}

static void subscribe_callback_duplex_func(struct mosquitto *mosq, void *obj, int mid, int qos_count,
                                           const int *granted_qos) {
  subscribe_callback_sub_func(mosq, obj, mid, qos_count, granted_qos);
}

static void log_callback_duplex_func(struct mosquitto *mosq, void *obj, int level, const char *str) {
  UNUSED(mosq);
  UNUSED(obj);
  UNUSED(level);
  log_info(logger_id, "log: %s\n", str);
}

status_t duplex_callback_func_set(struct mosquitto *mosq) {
  if (mosq == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_MQTT_NULL));
    return SC_MQTT_NULL;
  }

  mosquitto_log_callback_set(mosq, log_callback_duplex_func);
  mosquitto_subscribe_callback_set(mosq, subscribe_callback_duplex_func);
  mosquitto_connect_v5_callback_set(mosq, connect_callback_duplex_func);
  mosquitto_disconnect_v5_callback_set(mosq, disconnect_callback_duplex_func);
  mosquitto_publish_v5_callback_set(mosq, publish_callback_duplex_func);
  mosquitto_message_v5_callback_set(mosq, message_callback_duplex_func);

  return SC_OK;
}
