#include "config.h"
#include "utils/logger_helper.h"

#define CONFIG_LOGGER_ID "TA"

static logger_id_t logger_id;

status_t ta_config_init(ta_config_t* const info, iota_config_t* const tangle,
                        iota_client_service_t* const service) {
  status_t ret = SC_OK;
  if (info == NULL || tangle == NULL || service == NULL) {
    return SC_TA_NULL;
  }

  logger_id = logger_helper_enable(CONFIG_LOGGER_ID, LOGGER_DEBUG, true);
  log_info(logger_id, "[%s:%d] enable logger %s.\n", __func__, __LINE__,
           CONFIG_LOGGER_ID);

  log_info(logger_id, "Initializing TA information\n");
  info->version = TA_VERSION;
  info->host = TA_HOST;
  info->port = TA_PORT;
  info->thread_count = TA_THREAD_COUNT;

  log_info(logger_id, "Initializing IRI configuration\n");
  tangle->depth = DEPTH;
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
  if (iota_client_core_init(service)) {
    log_critical(logger_id, "Initializing IRI connection failed!\n");
    ret = SC_TA_OOM;
  }
  iota_client_extended_init();

  return ret;
}

void ta_config_destroy(iota_client_service_t* const service) {
  log_info(logger_id, "Destroying IRI connection\n");
  iota_client_extended_destroy();
  iota_client_core_destroy(service);
  logger_helper_release(logger_id);
}
