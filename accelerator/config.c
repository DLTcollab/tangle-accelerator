/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "config.h"
#include "utils/logger_helper.h"
#include "utils/macros.h"
#include "yaml.h"

#define CONFIG_LOGGER "TA"

static logger_id_t config_logger_id;

int get_conf_key(char const* const key) {
  for (int i = 0; i < cli_cmd_num; ++i) {
    if (!strcmp(ta_cli_arguments_g[i].name, key)) {
      return ta_cli_arguments_g[i].val;
    }
  }

  return 0;
}

struct option* cli_build_options() {
  struct option* long_options = (struct option*)malloc(cli_cmd_num * sizeof(struct option));
  for (int i = 0; i < cli_cmd_num; ++i) {
    long_options[i].name = ta_cli_arguments_g[i].name;
    long_options[i].has_arg = ta_cli_arguments_g[i].has_arg;
    long_options[i].flag = NULL;
    long_options[i].val = ta_cli_arguments_g[i].val;
  }
  return long_options;
}

status_t cli_config_set(char* conf_file, ta_config_t* const info, iota_config_t* const iconf, ta_cache_t* const cache,
                        iota_client_service_t* const service, int key, char* const value) {
  if (value == NULL || info == NULL || iconf == NULL || cache == NULL || service == NULL) {
    log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_NULL");
    return SC_CONF_NULL;
  }

  switch (key) {
    // TA configuration
    case TA_HOST_CLI:
      info->host = value;
      break;
    case TA_PORT_CLI:
      info->port = value;
      break;
    case TA_THREAD_COUNT_CLI:
      info->thread_count = atoi(value);
      break;

    // IRI configuration
    case IRI_HOST_CLI:
      service->http.host = value;
      break;
    case IRI_PORT_CLI:
      service->http.port = atoi(value);
      break;

    // Cache configuration
    case REDIS_HOST_CLI:
      cache->host = value;
      break;
    case REDIS_PORT_CLI:
      cache->port = atoi(value);
      break;

    // iconf IOTA configuration
    case MILESTONE_DEPTH_CLI:
      iconf->milestone_depth = atoi(value);
      break;
    case MWM_CLI:
      iconf->mwm = atoi(value);
      break;
    case SEED_CLI:
      iconf->seed = value;
      break;
    case CACHE:
      cache->cache_state = (toupper(value[0]) == 'T');
      break;

    // Verbose configuration
    case VERBOSE:
      verbose_mode = (toupper(value[0]) == 'T');
      break;

    // File configuration
    case CONF_CLI: {
      size_t arg_len = strlen(value);
      strncpy(conf_file, value, arg_len);
      conf_file[arg_len] = '\0';
      break;
    }
  }
  return SC_OK;
}

status_t ta_config_default_init(ta_config_t* const info, iota_config_t* const iconf, ta_cache_t* const cache,
                                iota_client_service_t* const service) {
  status_t ret = SC_OK;

  config_logger_id = logger_helper_enable(CONFIG_LOGGER, LOGGER_DEBUG, true);
  log_info(config_logger_id, "[%s:%d] enable logger %s.\n", __func__, __LINE__, CONFIG_LOGGER);

  if (info == NULL || iconf == NULL || cache == NULL || service == NULL) {
    log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_TA_NULL;
  }

  log_info(config_logger_id, "Initializing TA information\n");
  info->version = TA_VERSION;
  info->host = TA_HOST;
  info->port = TA_PORT;
  info->thread_count = TA_THREAD_COUNT;
#ifdef ENABLE_MQTT
  info->mqtt_host = MQTT_HOST;
  info->mqtt_topic_root = TOPIC_ROOT;
#endif
  log_info(config_logger_id, "Initializing Redis information\n");
  cache->host = REDIS_HOST;
  cache->port = REDIS_PORT;
  cache->cache_state = false;

  log_info(config_logger_id, "Initializing IRI configuration\n");
  iconf->milestone_depth = MILESTONE_DEPTH;
  iconf->mwm = MWM;
  iconf->seed = SEED;
  iconf->mam_file_path = tempnam(MAM_FILE_DIR, MAM_FILE_PREFIX);

  log_info(config_logger_id, "Initializing IRI connection\n");
  service->http.path = "/";
  service->http.content_type = "application/json";
  service->http.accept = "application/json";
  service->http.host = IRI_HOST;
  service->http.port = IRI_PORT;
  service->http.api_version = 1;
  service->serializer_type = SR_JSON;

  // Turn off verbose mode default
  verbose_mode = false;

  return ret;
}

status_t ta_config_file_init(ta_core_t* const conf, int argc, char** argv) {
  int key = 0;
  status_t ret = SC_OK;
  struct option* long_options = cli_build_options();
  yaml_parser_t parser;
  yaml_token_t token;
  FILE* file = NULL;
  char* arg = NULL;
  int state = 0;

  // Initialize default configuration file path with '\0'
  conf->conf_file[0] = '\0';

  if (!yaml_parser_initialize(&parser)) {
    ret = SC_CONF_PARSER_ERROR;
    log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_PARSER_ERROR");
    goto done;
  }

  // Loop through the CLI arguments for first time to find the configuration file path
  while ((key = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
    switch (key) {
      case ':':
        ret = SC_CONF_MISSING_ARGUMENT;
        log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_MISSING_ARGUMENT");
        break;
      case '?':
        ret = SC_CONF_UNKNOWN_OPTION;
        log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_UNKNOWN_OPTION");
        break;
      case CONF_CLI:
        ret = cli_config_set(conf->conf_file, &conf->info, &conf->iconf, &conf->cache, &conf->service, key, optarg);
        break;
      default:
        break;
    }
    if (ret != SC_OK) {
      break;
    }
  }

  // Reset the CLI option index for the second loop where they are actually analyzed
  optind = 1;

  if (strlen(conf->conf_file) == 0) {
    /* No configuration file specified */
    ret = SC_OK;
    goto done;
  }

  if ((file = fopen(conf->conf_file, "r")) == NULL) {
    /* The specified configuration file does not exist */
    ret = SC_CONF_FOPEN_ERROR;
    log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_FOPEN_ERROR");
    goto done;
  }

  yaml_parser_set_input_file(&parser, file);

  do { /* start reading tokens */
    if (!yaml_parser_scan(&parser, &token)) {
      ret = SC_CONF_PARSER_ERROR;
      log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_PARSER_ERROR");
      goto done;
    }
    switch (token.type) {
      case YAML_KEY_TOKEN:
        state = 0;
        break;
      case YAML_VALUE_TOKEN:
        state = 1;
        break;
      case YAML_SCALAR_TOKEN:
        arg = (char*)token.data.scalar.value;
        if (state == 0) {  // Key
          key = get_conf_key(arg);
        } else {  // Value
          if ((ret = cli_config_set(conf->conf_file, &conf->info, &conf->iconf, &conf->cache, &conf->service, key,
                                    strdup(arg))) != SC_OK) {
            goto done;
          }
        }
        break;
      default:
        break;
    }
    if (token.type != YAML_STREAM_END_TOKEN) {
      yaml_token_delete(&token);
    }
  } while (token.type != YAML_STREAM_END_TOKEN);

done:
  yaml_token_delete(&token);
  yaml_parser_delete(&parser);
  if (file) {
    fclose(file);
  }
  free(long_options);

  return ret;
}

status_t ta_config_cli_init(ta_core_t* const conf, int argc, char** argv) {
  int key = 0;
  status_t ret = SC_OK;
  struct option* long_options = cli_build_options();

  while ((key = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
    switch (key) {
      case ':':
        ret = SC_CONF_MISSING_ARGUMENT;
        log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_MISSING_ARGUMENT");
        break;
      case '?':
        ret = SC_CONF_UNKNOWN_OPTION;
        log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CONF_UNKNOWN_OPTION");
        break;
      case 'h':
        ta_usage();
        exit(EXIT_SUCCESS);
      case 'v':
        printf("%s\n", TA_VERSION);
        exit(EXIT_SUCCESS);
      case VERBOSE:
        // Turn on verbose mode
        verbose_mode = true;

        // Enable backend_redis logger
        br_logger_init();
        break;
      case CONF_CLI:
        /* Already processed in ta_config_file_init() */
        break;
      default:
        ret = cli_config_set(conf->conf_file, &conf->info, &conf->iconf, &conf->cache, &conf->service, key, optarg);
        break;
    }
    if (ret != SC_OK) {
      break;
    }
  }

  free(long_options);
  return ret;
}

status_t ta_config_set(ta_cache_t* const cache, iota_client_service_t* const service) {
  status_t ret = SC_OK;
  if (cache == NULL || service == NULL) {
    log_error(config_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_TA_NULL");
    return SC_TA_NULL;
  }

  if (iota_client_core_init(service)) {
    log_critical(config_logger_id, "Initializing IRI connection failed!\n");
    ret = SC_TA_OOM;
  }
  iota_client_extended_init();

  log_info(config_logger_id, "Initializing PoW implementation context\n");
  pow_init();

  log_info(config_logger_id, "Initializing cache state\n");
  cache_init(cache->cache_state, cache->host, cache->port);

  return ret;
}

void ta_config_destroy(iota_client_service_t* const service) {
  log_info(config_logger_id, "Destroying IRI connection\n");
  iota_client_extended_destroy();
  iota_client_core_destroy(service);

  pow_destroy();
  cache_stop();
  logger_helper_release(config_logger_id);
  br_logger_release();
}
