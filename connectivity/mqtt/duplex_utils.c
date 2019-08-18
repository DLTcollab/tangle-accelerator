/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "duplex_utils.h"
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include "utils/logger_helper.h"

#define MQTT_UTILS_LOGGER "mqtt-utils"

static logger_id_t mqtt_utils_logger_id;

void mqtt_utils_logger_init() { mqtt_utils_logger_id = logger_helper_enable(MQTT_UTILS_LOGGER, LOGGER_DEBUG, true); }

int mqtt_utils_logger_release() {
  logger_helper_release(mqtt_utils_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(mqtt_utils_logger_id, "[%s:%s] Destroying logger failed %s.\n", __func__, __LINE__, MQTT_UTILS_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t duplex_config_init(struct mosquitto **config_mosq, mosq_config_t *config_cfg) {
  status_t ret = SC_OK;
  if (config_mosq == NULL || config_cfg == NULL) {
    log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  init_mosq_config(config_cfg, client_duplex);
  mosquitto_lib_init();

  ret = generate_client_id(config_cfg);
  if (ret) {
    return ret;
  }

  init_check_error(config_cfg, client_pub);

  *config_mosq = mosquitto_new(config_cfg->general_config->id, true, NULL);
  if (!config_mosq) {
    switch (errno) {
      case ENOMEM:
        log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "Out of memory");
        break;
      case EINVAL:
        log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "Invalid id");
        break;
    }
    return SC_MOSQ_OBJ_INIT_ERROR;
  }

  ret = mosq_opts_set(*config_mosq, config_cfg);

  return ret;
}

status_t gossip_channel_set(mosq_config_t *channel_cfg, char *host, char *sub_topic, char *pub_topic) {
  status_t ret = SC_OK;
  if (channel_cfg == NULL || (host == NULL && sub_topic == NULL && pub_topic == NULL)) {
    log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  if (host) {
    channel_cfg->general_config->host = strdup(host);
    channel_cfg->general_config->client_type = client_pub;
  }

  if (sub_topic) {
    ret = cfg_add_topic(channel_cfg, client_sub, sub_topic);
    if (ret) {
      goto done;
    }
  }

  if (pub_topic) {
    ret = cfg_add_topic(channel_cfg, client_pub, pub_topic);
  }

done:
  return ret;
}

status_t gossip_api_channels_set(mosq_config_t *channel_cfg, char *host, char *root_path) {
  status_t ret = SC_OK;

  if (channel_cfg == NULL || host == NULL || root_path == NULL) {
    log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  channel_cfg->general_config->host = strdup(host);
  channel_cfg->general_config->client_type = client_pub;

  char *sub_topic = NULL;
  int sub_topic_len, api_name_len;
  int root_path_len = strlen(root_path);
  char *api_names[API_NUM] = {"address",          "tag/hashes", "tag/object", "transaction/object",
                              "transaction/send", "tips/all",   "tips/pair"};

  for (int i = 0; i < API_NUM; i++) {
    api_name_len = strlen(api_names[i]);
    sub_topic_len = root_path_len + 1 + api_name_len;
    sub_topic = (char *)malloc(sub_topic_len + 1);
    if (sub_topic == NULL) {
      ret = SC_MQTT_OOM;
      goto done;
    }

    snprintf(sub_topic, sub_topic_len + 1, "%s/%s", root_path, api_names[i]);

    ret = gossip_channel_set(channel_cfg, NULL, sub_topic, NULL);
    if (ret) {
      goto done;
    }

    free(sub_topic);
    sub_topic = NULL;
  }

done:
  free(sub_topic);
  return ret;
}

status_t gossip_message_set(mosq_config_t *cfg, char *message) {
  if (cfg == NULL || message == NULL) {
    log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  cfg->pub_config->message = strdup(message);
  cfg->pub_config->msglen = strlen(cfg->pub_config->message);

  return SC_OK;
}

status_t duplex_client_start(struct mosquitto *mosq, mosq_config_t *cfg) {
  status_t ret = MOSQ_ERR_SUCCESS;
  if (mosq == NULL || cfg == NULL) {
    log_error(mqtt_utils_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_MQTT_NULL;
  }

  cfg->general_config->client_type = client_sub;
  ret = mosq_client_connect(mosq, cfg);
  if (ret) {
    goto done;
  }

  ret = mosquitto_loop_forever(mosq, -1, 1);
  if (ret) {
    goto done;
  }

  cfg->general_config->client_type = client_pub;
  ret = mosq_client_connect(mosq, cfg);
  if (ret) {
    goto done;
  }
  ret = publish_loop(mosq);

done:
  return ret;
}
