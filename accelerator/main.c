#include "accelerator/errors.h"
#include "accelerator/http.h"
#include "utils/handles/signal.h"
#include "utils/logger_helper.h"

#define MAIN_LOGGER_ID "main"

static ta_core_t ta_core;
static ta_http_t ta_http;
static logger_id_t logger_id;

static void ta_stop(int signal) {
  if (signal == SIGINT || signal == SIGTERM) {
    ta_http_stop(&ta_http);
  }
}

int main(int argc, char* argv[]) {
  if (signal_handle_register(SIGINT, ta_stop) == SIG_ERR ||
      signal_handle_register(SIGTERM, ta_stop) == SIG_ERR) {
    return EXIT_FAILURE;
  }

  if (logger_helper_init() != RC_OK) {
    return EXIT_FAILURE;
  }
  logger_id = logger_helper_enable(MAIN_LOGGER_ID, LOGGER_DEBUG, true);

  // Initialize configurations with default value
  if (ta_config_default_init(&ta_core.info, &ta_core.tangle, &ta_core.cache,
                             &ta_core.service) != SC_OK) {
    return EXIT_FAILURE;
  }

  // Initialize configurations with CLI value
  if (ta_config_cli_init(&ta_core, argc, argv) != SC_OK) {
    return EXIT_FAILURE;
  }

  if (ta_config_set(&ta_core.cache, &ta_core.service) != SC_OK) {
    log_critical(logger_id, "[%s:%d] Configure failed %s.\n", __func__,
                 __LINE__, MAIN_LOGGER_ID);
    return EXIT_FAILURE;
  }

  if (ta_http_init(&ta_http, &ta_core) != SC_OK) {
    log_critical(logger_id, "[%s:%d] HTTP initialization failed %s.\n",
                 __func__, __LINE__, MAIN_LOGGER_ID);
    return EXIT_FAILURE;
  }

  if (ta_http_start(&ta_http) != SC_OK) {
    log_critical(logger_id, "[%s:%d] Starting TA failed %s.\n", __func__,
                 __LINE__, MAIN_LOGGER_ID);
    goto cleanup;
  }

  log_warning(logger_id, "Tangle-accelerator starts running\n");
  while (ta_http.running) {
    ;
  }

cleanup:
  log_warning(logger_id, "Destroying TA configurations\n");
  ta_config_destroy(&ta_core.service);
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(logger_id, "[%s:%d] Destroying logger failed %s.\n", __func__,
                 __LINE__, MAIN_LOGGER_ID);
    return EXIT_FAILURE;
  }

  return 0;
}
