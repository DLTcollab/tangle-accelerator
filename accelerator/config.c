#include "config.h"
#include "utils/logger_helper.h"

#define CONFIG_LOGGER_ID "TA"

static logger_id_t logger_id;

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

status_t cli_config_set(ta_config_t* const info, iota_config_t* const tangle, ta_cache_t* const cache,
                        iota_client_service_t* const service, int key, char* const value) {
  if (value == NULL || info == NULL || tangle == NULL || cache == NULL || service == NULL) {
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

    // Tangle configuration
    case MILESTONE_DEPTH_CLI:
      tangle->milestone_depth = atoi(value);
      break;
    case MWM_CLI:
      tangle->mwm = atoi(value);
      break;
    case SEED_CLI:
      tangle->seed = value;
      break;
  }
  return SC_OK;
}

status_t ta_config_default_init(ta_config_t* const info, iota_config_t* const tangle, ta_cache_t* const cache,
                                iota_client_service_t* const service) {
  status_t ret = SC_OK;
  char mss_tmp[] = "/tmp/XXXXXX";
  if (info == NULL || tangle == NULL || cache == NULL || service == NULL) {
    return SC_TA_NULL;
  }

  logger_id = logger_helper_enable(CONFIG_LOGGER_ID, LOGGER_DEBUG, true);
  log_info(logger_id, "[%s:%d] enable logger %s.\n", __func__, __LINE__, CONFIG_LOGGER_ID);

  log_info(logger_id, "Initializing TA information\n");
  info->version = TA_VERSION;
  info->host = TA_HOST;
  info->port = TA_PORT;
  info->thread_count = TA_THREAD_COUNT;

  log_info(logger_id, "Initializing Redis information\n");
  cache->host = REDIS_HOST;
  cache->port = REDIS_PORT;

  log_info(logger_id, "Initializing IRI configuration\n");
  tangle->milestone_depth = MILESTONE_DEPTH;
  tangle->mwm = MWM;
  tangle->seed = SEED;

  log_info(logger_id, "Initializing IRI connection\n");
  service->http.path = "/";
  service->http.content_type = "application/json";
  service->http.accept = "application/json";
  service->http.host = IRI_HOST;
  service->http.port = IRI_PORT;
  service->http.api_version = 1;
  service->serializer_type = SR_JSON;
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
        break;
      case '?':
        ret = SC_CONF_UNKNOWN_OPTION;
        break;
      case 'h':
        ta_usage();
        exit(EXIT_SUCCESS);
      case 'v':
        printf("%s\n", TA_VERSION);
        exit(EXIT_SUCCESS);
      default:
        ret = cli_config_set(&conf->info, &conf->tangle, &conf->cache, &conf->service, key, optarg);
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
    return SC_TA_NULL;
  }

  if (iota_client_core_init(service)) {
    log_critical(logger_id, "Initializing IRI connection failed!\n");
    ret = SC_TA_OOM;
  }
  iota_client_extended_init();

  log_info(logger_id, "Initializing PoW implementation context\n");
  pow_init();

  log_info(logger_id, "Initializing cache connection\n");
  cache_init(cache->host, cache->port);
  return ret;
}

void ta_config_destroy(iota_client_service_t* const service) {
  log_info(logger_id, "Destroying IRI connection\n");
  iota_client_extended_destroy();
  iota_client_core_destroy(service);

  pow_destroy();
  cache_stop();
  logger_helper_release(logger_id);
}
