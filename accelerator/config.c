/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "config.h"
#include "utils/macros.h"
#include "yaml.h"

#define CONFIG_LOGGER "config"

static logger_id_t logger_id;

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
    long_options[i].flag = ta_cli_arguments_g[i].flag;
    long_options[i].val = ta_cli_arguments_g[i].val;
  }
  return long_options;
}

static status_t cli_core_set(ta_core_t* const core, int key, char* const value) {
  if (value == NULL && (key != PROXY_API && key != GTTA_DISABLE)) {
    ta_log_error("%s\n", "SC_CONF_NULL");
    return SC_CONF_NULL;
  }
  ta_config_t* const ta_conf = &core->ta_conf;
  iota_config_t* const iota_conf = &core->iota_conf;
  ta_cache_t* const cache = &core->cache;
  iota_client_service_t* const iota_service = &core->iota_service;
  char* conf_file = core->conf_file;
#ifdef DB_ENABLE
  db_client_service_t* const db_service = &core->db_service;
#endif
  switch (key) {
    // TA configuration
    case TA_HOST_CLI:
      ta_conf->host = value;
      break;
    case TA_PORT_CLI:
      ta_conf->port = value;
      break;
    case TA_THREAD_COUNT_CLI:
      ta_conf->thread_count = atoi(value);
      break;

    // IRI configuration
    case IRI_HOST_CLI:
      iota_service->http.host = value;
      break;
    case IRI_PORT_CLI:
      iota_service->http.port = atoi(value);
      break;

#ifdef MQTT_ENABLE
    // MQTT configuration
    case MQTT_HOST_CLI:
      ta_conf->mqtt_host = value;
      break;
    case MQTT_ROOT_CLI:
      ta_conf->mqtt_topic_root = value;
      break;
#endif

    // Cache configuration
    case REDIS_HOST_CLI:
      cache->host = value;
      break;
    case REDIS_PORT_CLI:
      cache->port = atoi(value);
      break;
#ifdef DB_ENABLE
    // DB configuration
    case DB_HOST_CLI:
      free(db_service->host);
      db_service->host = strdup(value);
      break;
#endif
    // iota_conf IOTA configuration
    case MILESTONE_DEPTH_CLI:
      iota_conf->milestone_depth = atoi(value);
      break;
    case MWM_CLI:
      iota_conf->mwm = atoi(value);
      break;
    case SEED_CLI:
      iota_conf->seed = value;
      break;
    case CACHE:
      cache->cache_state = (toupper(value[0]) == 'T');
      break;

    // Quiet mode configuration
    case QUIET:
      quiet_mode = (toupper(value[0]) == 'T');
      break;
    case PROXY_API:
      ta_conf->proxy_passthrough = true;
      break;
    case GTTA_DISABLE:
      ta_conf->gtta_disable = true;
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

status_t ta_core_default_init(ta_core_t* const core) {
  status_t ret = SC_OK;

  logger_id = logger_helper_enable(CONFIG_LOGGER, LOGGER_DEBUG, true);
  ta_log_info("enable logger %s.\n", CONFIG_LOGGER);

  ta_config_t* const ta_conf = &core->ta_conf;
  iota_config_t* const iota_conf = &core->iota_conf;
  ta_cache_t* const cache = &core->cache;
  iota_client_service_t* const iota_service = &core->iota_service;
#ifdef DB_ENABLE
  db_client_service_t* const db_service = &core->db_service;
#endif

  ta_log_info("Initializing TA information\n");
  ta_conf->version = TA_VERSION;
  ta_conf->host = TA_HOST;
  ta_conf->port = TA_PORT;
  ta_conf->thread_count = TA_THREAD_COUNT;
  ta_conf->proxy_passthrough = false;
  ta_conf->gtta_disable = false;
#ifdef MQTT_ENABLE
  ta_conf->mqtt_host = MQTT_HOST;
  ta_conf->mqtt_topic_root = TOPIC_ROOT;
#endif
  ta_log_info("Initializing Redis information\n");
  cache->host = REDIS_HOST;
  cache->port = REDIS_PORT;
  cache->cache_state = false;

  ta_log_info("Initializing IRI configuration\n");
  iota_conf->milestone_depth = MILESTONE_DEPTH;
  iota_conf->mwm = MWM;
  iota_conf->seed = SEED;
  char mam_file_path[] = MAM_FILE_PREFIX;
  mkstemp(mam_file_path);
  iota_conf->mam_file_path = mam_file_path;

  ta_log_info("Initializing IRI connection\n");
  iota_service->http.path = "/";
  iota_service->http.content_type = "application/json";
  iota_service->http.accept = "application/json";
  iota_service->http.host = IRI_HOST;
  iota_service->http.port = IRI_PORT;
  iota_service->http.api_version = 1;
  iota_service->serializer_type = SR_JSON;
#ifdef DB_ENABLE
  ta_log_info("Initializing DB connection\n");
  db_service->host = strdup(DB_HOST);
#endif
  // Turn off quiet mode default
  quiet_mode = false;

  return ret;
}

status_t ta_core_file_init(ta_core_t* const core, int argc, char** argv) {
  int key = 0;
  status_t ret = SC_OK;
  struct option* long_options = cli_build_options();
  yaml_parser_t parser;
  yaml_token_t token;
  FILE* file = NULL;
  char* arg = NULL;
  int state = 0;

  // Initialize default configuration file path with '\0'
  core->conf_file[0] = '\0';

  if (!yaml_parser_initialize(&parser)) {
    ret = SC_CONF_PARSER_ERROR;
    ta_log_error("%s\n", "SC_CONF_PARSER_ERROR");
    goto done;
  }

  // Loop through the CLI arguments for first time to find the configuration file path
  while ((key = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
    switch (key) {
      case ':':
        ret = SC_CONF_MISSING_ARGUMENT;
        ta_log_error("%s\n", "SC_CONF_MISSING_ARGUMENT");
        break;
      case '?':
        ret = SC_CONF_UNKNOWN_OPTION;
        ta_log_error("%s\n", "SC_CONF_UNKNOWN_OPTION");
        continue;
      case CONF_CLI:
        ret = cli_core_set(core, key, optarg);
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

  if (strlen(core->conf_file) == 0) {
    /* No configuration file specified */
    ret = SC_OK;
    goto done;
  }

  if ((file = fopen(core->conf_file, "r")) == NULL) {
    /* The specified configuration file does not exist */
    ret = SC_CONF_FOPEN_ERROR;
    ta_log_error("%s\n", "SC_CONF_FOPEN_ERROR");
    goto done;
  }

  yaml_parser_set_input_file(&parser, file);

  do { /* start reading tokens */
    if (!yaml_parser_scan(&parser, &token)) {
      ret = SC_CONF_PARSER_ERROR;
      ta_log_error("%s\n", "SC_CONF_PARSER_ERROR");
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
          if ((ret = cli_core_set(core, key, strdup(arg))) != SC_OK) {
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

status_t ta_core_cli_init(ta_core_t* const core, int argc, char** argv) {
  int key = 0;
  status_t ret = SC_OK;
  struct option* long_options = cli_build_options();

  while ((key = getopt_long(argc, argv, "hv", long_options, NULL)) != -1) {
    switch (key) {
      case ':':
        ret = SC_CONF_MISSING_ARGUMENT;
        ta_log_error("%s\n", "SC_CONF_MISSING_ARGUMENT");
        break;
      case '?':
        ret = SC_CONF_UNKNOWN_OPTION;
        ta_log_error("%s\n", "SC_CONF_UNKNOWN_OPTION");
        continue;
      case 'h':
        ta_usage();
        exit(EXIT_SUCCESS);
      case 'v':
        printf("%s\n", TA_VERSION);
        exit(EXIT_SUCCESS);
      case QUIET:
        // Turn on quiet mode
        quiet_mode = true;

        // Enable backend_redis logger
        br_logger_init();
        break;
      case CONF_CLI:
        /* Already processed in ta_config_file_init() */
        break;
      default:
        ret = cli_core_set(core, key, optarg);
        break;
    }
    if (ret != SC_OK) {
      break;
    }
  }

  free(long_options);
  return ret;
}

status_t ta_core_set(ta_core_t* core) {
  status_t ret = SC_OK;

  ta_cache_t* const cache = &core->cache;
  iota_client_service_t* const iota_service = &core->iota_service;
#ifdef DB_ENABLE
  db_client_service_t* const db_service = &core->db_service;
#endif
  if (iota_client_core_init(iota_service)) {
    ta_log_error("Initializing IRI connection failed!\n");
    ret = SC_TA_OOM;
    goto exit;
  }
  iota_client_extended_init();

  ta_log_info("Initializing PoW implementation context\n");
  pow_init();

  ta_log_info("Initializing cache state\n");
  cache_init(cache->cache_state, cache->host, cache->port);
#ifdef DB_ENABLE
  ta_log_info("Initializing db client service\n");
  if ((ret = db_client_service_init(db_service, DB_USAGE_REATTACH)) != SC_OK) {
    ta_log_error("Initializing DB connection failed\n");
  }
#endif

exit:
  return ret;
}

void ta_core_destroy(ta_core_t* const core) {
  ta_log_info("Destroying IRI connection\n");
  iota_client_extended_destroy();
  iota_client_core_destroy(&core->iota_service);
#ifdef DB_ENABLE
  ta_log_info("Destroying DB connection\n");
  db_client_service_free(&core->db_service);
#endif

  pow_destroy();
  cache_stop();
  logger_helper_release(logger_id);
  br_logger_release();
}
