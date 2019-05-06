#include "accelerator/errors.h"
#include "accelerator/http.h"
#include "utils/logger_helper.h"

#define MAIN_LOGGER_ID "main"

static ta_core_t ta_core;
static ta_http_t ta_http;
static logger_id_t logger_id;

int main(int argc, char* argv[]) {
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

  status_t ret = SC_OK;
  ret = ta_http_init(&ta_http, &ta_core);
  if (ret) {
    return EXIT_FAILURE;
  }

  ret = ta_http_start(&ta_http);
  if (ret) {
    ta_http_stop(&ta_http);
  }

  // TODO: Discuss when will TA force to stop, stop by pressing Enter is
  // temporary
  printf("Tangle-accelerator starts running, press Enter to stop\n");
  (void)getc(stdin);

  ta_http_stop(&ta_http);
  ta_config_destroy(&ta_core.service);
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    return EXIT_FAILURE;
  }

  return 0;
}
