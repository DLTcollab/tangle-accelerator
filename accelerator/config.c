/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "config.h"
#include <errno.h>
#include <limits.h>
#include "utils/macros.h"
#include "yaml.h"

#define CONFIG_LOGGER "config"

static logger_id_t logger_id;

/**
 * @brief Get the corresponding value of the key from ta_cli_arguments_g structure
 * key : correspond to "name" in ta_cli_arguments_g structure
 * value : correspond to "val" in ta_cli_arguments_g structure
 *
 * @param key[in] Key of the key-value pair in yaml file
 *
 * @return
 * - ZERO on Parsing unknown key
 * - non-zero Corresponding value of key
 */
static int get_conf_key(char const* const key) {
  for (int i = 0; i < cli_cmd_num - 1; i++) {
    if (!strcmp(ta_cli_arguments_g[i].name, key)) {
      return ta_cli_arguments_g[i].val;
    }
  }
  ta_log_error("Invalid %s setting in the configuration file\n", key);
  return 0;
}

status_t cli_core_set(ta_core_t* const core, int key, char* const value) {
  if (value == NULL && (key != CACHE && key != PROXY_API && key != QUIET && key != NO_GTTA)) {
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
  char* strtol_p = NULL;
  long int strtol_temp;
  uint8_t idx;
  switch (key) {
    // TA configuration
    case TA_HOST_CLI:
      ta_conf->host = value;
      break;
    case TA_PORT_CLI:
      strtol_temp = strtol(value, &strtol_p, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
        ta_conf->port = (int)strtol_temp;
      } else {
        ta_log_error("Malformed input\n");
      }
      break;
    case HTTP_THREADS_CLI:
      strtol_temp = strtol(value, &strtol_p, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= 0 && strtol_temp <= UCHAR_MAX) {
        ta_log_debug("Number of logical processors : %d, physical processors : %d\n", get_nprocs_conf(),
                     get_nprocs_conf() / get_nthds_per_phys_proc());
        if ((uint8_t)strtol_temp > MAX_HTTP_TPOOL_SIZE) {
          ta_log_warning("Requiring thread number %d exceed limitation. Set it to the maximum allowed number: %d\n",
                         (uint8_t)strtol_temp, MAX_HTTP_TPOOL_SIZE);
          ta_conf->http_tpool_size = (uint8_t)MAX_HTTP_TPOOL_SIZE;
        } else {
          ta_conf->http_tpool_size = (uint8_t)strtol_temp;
        }
      } else {
        ta_log_error("Malformed input\n");
      }
      break;

    // IRI configuration
    case IRI_HOST_CLI:
      idx = 0;
      for (char* p = strtok(value, ","); p && idx < MAX_IRI_LIST_ELEMENTS; p = strtok(NULL, ","), idx++) {
        ta_conf->iota_host_list[idx] = p;
      }
      strncpy(iota_service->http.host, ta_conf->iota_host_list[0], HOST_MAX_LEN);
      break;
    case IRI_PORT_CLI:
      idx = 0;
      for (char* p = strtok(value, ","); p && idx < MAX_IRI_LIST_ELEMENTS; p = strtok(NULL, ","), idx++) {
        strtol_temp = strtol(p, &strtol_p, 10);
        if (strtol_p != p && errno != ERANGE && strtol_temp >= 0 && strtol_temp <= USHRT_MAX) {
          ta_conf->iota_port_list[idx] = (uint16_t)strtol_temp;
        } else {
          ta_log_error("Malformed input\n");
        }
      }
      iota_service->http.port = ta_conf->iota_port_list[0];
      break;
    case HEALTH_TRACK_PERIOD:
      strtol_temp = strtol(value, NULL, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
        ta_conf->health_track_period = (int)strtol_temp;
      } else {
        ta_log_error("Malformed input\n");
      }
      break;
    case CACHE:
      ta_log_info("Initializing cache state\n");
      cache->state = !cache->state;
      if (cache->state) {
        if (cache_init(&cache->rwlock, cache->state, cache->host, cache->port)) {
          ta_log_error("%s\n", "Failed to initialize lock to caching service.");
        }
      }

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
      strtol_temp = strtol(value, NULL, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
        cache->port = (int)strtol_temp;
      } else {
        ta_log_error("Malformed input character\n");
      }
      break;
    case CACHE_MAX_CAPACITY:
      strtol_temp = strtol(value, NULL, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
        if (strtol_temp <= 0) {
          ta_log_error("The capacity of caching service should greater then 0.\n");
          break;
        }
        cache->capacity = strtol_temp;
      } else {
        ta_log_error("Malformed input\n");
      }
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
      strtol_temp = strtol(value, NULL, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
        iota_conf->milestone_depth = (int)strtol_temp;
      } else {
        ta_log_error("Malformed input\n");
      }
      break;
    case MWM_CLI:
      strtol_temp = strtol(value, NULL, 10);
      if (strtol_p != value && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
        iota_conf->mwm = (int)strtol_temp;
      } else {
        ta_log_error("Malformed input\n");
      }
      break;
    case SEED_CLI:
      iota_conf->seed = value;
      break;

    // Quiet mode configuration
    case QUIET:
      quiet_mode = true;
      break;
    case PROXY_API:
      ta_conf->proxy_passthrough = true;
      break;
    case NO_GTTA:
      ta_conf->gtta = false;
      break;
    case BUFFER_LIST:
      cache->buffer_list_name = value;
      break;
    case DONE_LIST:
      cache->done_list_name = value;
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

status_t ta_set_iota_client_service(iota_client_service_t* service, char const* host, uint16_t port) {
  strncpy(service->http.path, "/", CONTENT_TYPE_MAX_LEN);
  strncpy(service->http.content_type, "application/json", CONTENT_TYPE_MAX_LEN);
  strncpy(service->http.accept, "application/json", CONTENT_TYPE_MAX_LEN);
  strncpy(service->http.host, host, HOST_MAX_LEN);
  service->http.port = port;
  service->http.api_version = 1;
  service->http.ca_pem = NULL;
  service->serializer_type = SR_JSON;
  init_json_serializer(&service->serializer);

  return SC_OK;
}

status_t ta_core_default_init(ta_core_t* const core) {
  status_t ret = SC_OK;

  logger_id = logger_helper_enable(CONFIG_LOGGER, LOGGER_DEBUG, true);
  ta_log_info("Enable logger %s.\n", CONFIG_LOGGER);

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
  memset(ta_conf->iota_host_list, 0, sizeof(ta_conf->iota_host_list));
  for (int i = 0; i < MAX_IRI_LIST_ELEMENTS; i++) {
    ta_conf->iota_port_list[i] = IRI_PORT;
  }
  ta_conf->http_tpool_size = DEFAULT_HTTP_TPOOL_SIZE;
  ta_conf->proxy_passthrough = false;
  ta_conf->health_track_period = HEALTH_TRACK_PERIOD;
  ta_conf->gtta = true;
#ifdef MQTT_ENABLE
  ta_conf->mqtt_host = MQTT_HOST;
  ta_conf->mqtt_topic_root = TOPIC_ROOT;
#endif
  ta_log_info("Initializing Redis information\n");
  cache->host = REDIS_HOST;
  cache->port = REDIS_PORT;
  cache->state = false;
  cache->buffer_list_name = BUFFER_LIST_NAME;
  cache->done_list_name = DONE_LIST_NAME;
  cache->capacity = CACHE_MAX_CAPACITY;

  ta_log_info("Initializing IRI configuration\n");
  iota_conf->milestone_depth = MILESTONE_DEPTH;
  iota_conf->mwm = MWM;
  iota_conf->seed = SEED;
  char mam_file_path[] = MAM_FILE_PREFIX;
  mkstemp(mam_file_path);
  iota_conf->mam_file_path = strdup(mam_file_path);

  ta_log_info("Initializing IRI connection\n");
  strncpy(iota_service->http.path, "/", CONTENT_TYPE_MAX_LEN);
  strncpy(iota_service->http.content_type, "application/json", CONTENT_TYPE_MAX_LEN);
  strncpy(iota_service->http.accept, "application/json", CONTENT_TYPE_MAX_LEN);
  strncpy(iota_service->http.host, IRI_HOST, HOST_MAX_LEN);
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

  iota_client_service_t* const iota_service = &core->iota_service;
#ifdef DB_ENABLE
  db_client_service_t* const db_service = &core->db_service;
#endif
  if (iota_client_service_init(iota_service)) {
    ta_log_error("Initializing IRI connection failed!\n");
    ret = SC_TA_OOM;
    goto exit;
  }

  // Initialize logger in 'iota.c'
  logger_helper_init(LOGGER_DEBUG);
  logger_init_client_core(LOGGER_DEBUG);
  logger_init_client_extended(LOGGER_DEBUG);
  logger_init_json_serializer(LOGGER_DEBUG);

  ta_log_info("Initializing PoW implementation context\n");
  pow_init();

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
#ifdef DB_ENABLE
  ta_log_info("Destroying DB connection\n");
  db_client_service_free(&core->db_service);
#endif
  pow_destroy();
  cache_stop(&core->cache.rwlock);
  logger_helper_release(logger_id);
  logger_destroy_client_core();
  logger_destroy_client_extended();
  logger_destroy_json_serializer();
  br_logger_release();
}
