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
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>
#include "utils/macros.h"
#include "yaml.h"

#define CONFIG_LOGGER "config"
#define DEFAULT_CA_PEM                                                   \
  "-----BEGIN CERTIFICATE-----\r\n"                                      \
  "MIIDQTCCAimgAwIBAgITBmyfz5m/jAo54vB4ikPmljZbyjANBgkqhkiG9w0BAQsF\r\n" \
  "ADA5MQswCQYDVQQGEwJVUzEPMA0GA1UEChMGQW1hem9uMRkwFwYDVQQDExBBbWF6\r\n" \
  "b24gUm9vdCBDQSAxMB4XDTE1MDUyNjAwMDAwMFoXDTM4MDExNzAwMDAwMFowOTEL\r\n" \
  "MAkGA1UEBhMCVVMxDzANBgNVBAoTBkFtYXpvbjEZMBcGA1UEAxMQQW1hem9uIFJv\r\n" \
  "b3QgQ0EgMTCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBALJ4gHHKeNXj\r\n" \
  "ca9HgFB0fW7Y14h29Jlo91ghYPl0hAEvrAIthtOgQ3pOsqTQNroBvo3bSMgHFzZM\r\n" \
  "9O6II8c+6zf1tRn4SWiw3te5djgdYZ6k/oI2peVKVuRF4fn9tBb6dNqcmzU5L/qw\r\n" \
  "IFAGbHrQgLKm+a/sRxmPUDgH3KKHOVj4utWp+UhnMJbulHheb4mjUcAwhmahRWa6\r\n" \
  "VOujw5H5SNz/0egwLX0tdHA114gk957EWW67c4cX8jJGKLhD+rcdqsq08p8kDi1L\r\n" \
  "93FcXmn/6pUCyziKrlA4b9v7LWIbxcceVOF34GfID5yHI9Y/QCB/IIDEgEw+OyQm\r\n" \
  "jgSubJrIqg0CAwEAAaNCMEAwDwYDVR0TAQH/BAUwAwEB/zAOBgNVHQ8BAf8EBAMC\r\n" \
  "AYYwHQYDVR0OBBYEFIQYzIU07LwMlJQuCFmcx7IQTgoIMA0GCSqGSIb3DQEBCwUA\r\n" \
  "A4IBAQCY8jdaQZChGsV2USggNiMOruYou6r4lK5IpDB/G/wkjUu0yKGX9rbxenDI\r\n" \
  "U5PMCCjjmCXPI6T53iHTfIUJrU6adTrCC2qJeHZERxhlbI1Bjjt/msv0tadQ1wUs\r\n" \
  "N+gDS63pYaACbvXy8MWy7Vu33PqUXHeeE6V/Uq2V8viTO96LXFvKWlJbYK8U90vv\r\n" \
  "o/ufQJVtMVT8QtPHRh8jrdkPSHCa2XV4cdFyQzR1bldZwgJcJmApzyMZFo6IQ6XU\r\n" \
  "5MsI+yMRQ+hDKXJioaldXgjUkK642M4UwtBV8ob2xJNDd2ZhwLnoQdeXeGADbkpy\r\n" \
  "rqXRfboQnoZsG4q5WTP468SQvvG5\r\n"                                     \
  "-----END CERTIFICATE-----\r\n"

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

#define CA_BUFFER_SIZE 2048
static char* read_ca_pem(char* ca_pem_path) {
  FILE* file = NULL;
  if ((file = fopen(ca_pem_path, "r")) == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CONF_FOPEN_ERROR));
    goto done;
  }

  char* ca_pem = (char*)malloc(sizeof(char) * (CA_BUFFER_SIZE));
  if (!ca_pem) {
    ta_log_error("%s\n", ta_error_to_string(SC_OOM));
    goto done;
  }

  char c;
  int i = 0, buffer_nums = 1;
  while ((c = fgetc(file)) != EOF) {
    ca_pem[i++] = c;
    if (!(i < CA_BUFFER_SIZE)) {
      ca_pem = realloc(ca_pem, sizeof(char) * buffer_nums * CA_BUFFER_SIZE);
      buffer_nums++;
    }
  }
  ca_pem[i] = '\0';

  if (feof(file)) {
    ta_log_debug("Read the End Of File.\n");
  } else {
    ta_log_error("Read file error\n");
  }

done:
  fclose(file);
  return ca_pem;
}

status_t cli_core_set(ta_core_t* const core, int key, char* const value) {
  if (value == NULL) {
    switch (key) {
      /* No argument */
      case CACHE:
      case PROXY_API:
      case QUIET:
      case NO_GTTA:
      case RUNTIME_CLI:
        break;
      /* Need argument but no value found */
      default:
        ta_log_error("%s\n", "SC_NULL");
        return SC_NULL;
    }
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

    // IOTA full node configuration
    case NODE_HOST_CLI:
      idx = 0;
      for (char* p = strtok(value, ","); p && idx < MAX_NODE_LIST_ELEMENTS; p = strtok(NULL, ","), idx++) {
        ta_conf->iota_host_list[idx] = p;
      }
      strncpy(iota_service->http.host, ta_conf->iota_host_list[0], HOST_MAX_LEN - 1);
      break;
    case NODE_PORT_CLI:
      idx = 0;
      for (char* p = strtok(value, ","); p && idx < MAX_NODE_LIST_ELEMENTS; p = strtok(NULL, ","), idx++) {
        strtol_temp = strtol(p, &strtol_p, 10);
        if (strtol_p != p && errno != ERANGE && strtol_temp >= 0 && strtol_temp <= USHRT_MAX) {
          ta_conf->iota_port_list[idx] = (uint16_t)strtol_temp;
        } else {
          ta_log_error("Malformed input\n");
        }
      }
      iota_service->http.port = ta_conf->iota_port_list[0];
      break;

    case CA_PEM:
      iota_service->http.ca_pem = read_ca_pem(value);
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
        if (!cache_init(&cache->rwlock, cache->state, cache->host, cache->port)) {
          ta_log_error("%s\n", "Failed to initialize lock to caching service.");
        }
      }

      break;
    case IPC:
      ta_conf->socket = strdup(value);
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
    case CACHE_CAPACITY:
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
    case BUFFER_LIST:
      cache->buffer_list_name = value;
      break;
    case COMPLETE_LIST:
      cache->complete_list_name = value;
      break;

    // Command line options configuration
    case QUIET:
      ta_conf->cli_options |= CLI_QUIET_MODE;
      break;
    case PROXY_API:
      ta_conf->cli_options |= CLI_PROXY_PASSTHROUGH;
      break;
    case NO_GTTA:
      ta_conf->cli_options |= CLI_GTTA;
      break;
    case RUNTIME_CLI:
      ta_conf->cli_options |= CLI_RUNTIME_CLI;
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

status_t ta_set_iota_client_service(iota_client_service_t* service, char const* host, uint16_t port,
                                    char const* const ca_pem) {
  strncpy(service->http.path, "/", CONTENT_TYPE_MAX_LEN);
  strncpy(service->http.content_type, "application/json", CONTENT_TYPE_MAX_LEN - 1);
  strncpy(service->http.accept, "application/json", CONTENT_TYPE_MAX_LEN - 1);
  strncpy(service->http.host, host, HOST_MAX_LEN - 1);
  service->http.port = port;
  service->http.api_version = 1;
  service->http.ca_pem = ca_pem;
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
  for (int i = 0; i < MAX_NODE_LIST_ELEMENTS; i++) {
    ta_conf->iota_port_list[i] = NODE_PORT;
  }
  ta_conf->http_tpool_size = DEFAULT_HTTP_TPOOL_SIZE;
  ta_conf->health_track_period = HEALTH_TRACK_PERIOD;
  ta_conf->socket = DOMAIN_SOCKET;
#ifdef MQTT_ENABLE
  ta_conf->mqtt_host = MQTT_HOST;
  ta_conf->mqtt_topic_root = TOPIC_ROOT;
#endif
  ta_log_info("Initializing Redis information\n");
  cache->host = REDIS_HOST;
  cache->port = REDIS_PORT;
  cache->state = false;
  cache->buffer_list_name = BUFFER_LIST_NAME;
  cache->complete_list_name = COMPLETE_LIST_NAME;
  cache->mam_buffer_list_name = MAM_BUFFER_LIST_NAME;
  cache->mam_complete_list_name = MAM_COMPLETE_LIST_NAME;
  cache->capacity = CACHE_MAX_CAPACITY;

  ta_log_info("Initializing IOTA full node configuration\n");
  iota_conf->milestone_depth = MILESTONE_DEPTH;
  iota_conf->mwm = MWM;
  iota_conf->seed = SEED;
  char mam_file_path[] = MAM_FILE_PREFIX;
  mkstemp(mam_file_path);
  iota_conf->mam_file_path = strdup(mam_file_path);

  ta_log_info("Initializing IOTA full node connection\n");
  strncpy(iota_service->http.path, "/", CONTENT_TYPE_MAX_LEN);
  strncpy(iota_service->http.content_type, "application/json", CONTENT_TYPE_MAX_LEN);
  strncpy(iota_service->http.accept, "application/json", CONTENT_TYPE_MAX_LEN);
  strncpy(iota_service->http.host, NODE_HOST, HOST_MAX_LEN);
  iota_service->http.ca_pem = DEFAULT_CA_PEM;
  iota_service->http.port = NODE_PORT;
  iota_service->http.api_version = 1;
  iota_service->serializer_type = SR_JSON;
#ifdef DB_ENABLE
  ta_log_info("Initializing DB connection\n");
  db_service->host = strdup(DB_HOST);
#endif
  // Command line options set default to 0
  ta_conf->cli_options &= ~CLI_QUIET_MODE;
  ta_conf->cli_options &= ~CLI_RUNTIME_CLI;
  ta_conf->cli_options &= ~CLI_PROXY_PASSTHROUGH;

  // Command line options set default to 1
  ta_conf->cli_options |= CLI_GTTA;

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
    ta_log_error("Initializing IOTA full node connection failed!\n");
    ret = SC_OOM;
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
  ta_log_info("Destroying IOTA full node connection\n");
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
}

bool is_option_enabled(const ta_config_t* const ta_config, uint32_t option) {
  return (bool)(ta_config->cli_options & option);
}

void ta_logger_switch(bool quiet, bool init, ta_config_t* ta_conf) {
  if (quiet != is_option_enabled(ta_conf, CLI_QUIET_MODE) || init) {
    if (quiet == false) {
#ifdef MQTT_ENABLE
      mqtt_utils_logger_init();
      mqtt_common_logger_init();
      mqtt_callback_logger_init();
      mqtt_pub_logger_init();
      mqtt_sub_logger_init();
#else
      http_logger_init();
#endif
      conn_logger_init();
      apis_logger_init();
      cc_logger_init();
      serializer_logger_init();
      pow_logger_init();
      timer_logger_init();
      br_logger_init();
      ta_conf->cli_options &= ~CLI_QUIET_MODE;
    } else {
#ifdef MQTT_ENABLE
      mqtt_utils_logger_release();
      mqtt_common_logger_release();
      mqtt_callback_logger_release();
      mqtt_pub_logger_release();
      mqtt_sub_logger_release();
#else
      http_logger_release();
#endif
      conn_logger_release();
      apis_logger_release();
      cc_logger_release();
      serializer_logger_release();
      pow_logger_release();
      timer_logger_release();
      br_logger_release();
      ta_conf->cli_options |= CLI_QUIET_MODE;
    }
  }
}

#define START_NOTIFICATION "TA-START"
void notification_trigger(ta_config_t* const ta_conf) {
  int connect_fd;
  static struct sockaddr_un srv_addr;

  // Create UNIX domain socket
  connect_fd = socket(PF_UNIX, SOCK_STREAM, 0);
  if (connect_fd < 0) {
    ta_log_error("Can't create communication socket.\n");
    goto done;
  }

  srv_addr.sun_family = AF_UNIX;
  strncpy(srv_addr.sun_path, ta_conf->socket, strlen(ta_conf->socket));

  // Connect to UNIX domain socket server
  if (connect(connect_fd, (struct sockaddr*)&srv_addr, sizeof(srv_addr)) == -1) {
    ta_log_error("Can't connect to UNIX domain socket server.\n");
    goto done;
  }

  // Send notification to UNIX domain socket
  if (write(connect_fd, START_NOTIFICATION, strlen(START_NOTIFICATION)) == -1) {
    ta_log_error("Can't write message to UNIX domain socket server.\n");
  }

done:
  close(connect_fd);
  unlink(ta_conf->socket);
}
