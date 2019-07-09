#ifndef ACCELERATOR_HTTP_H_
#define ACCELERATOR_HTTP_H_

#include <stdbool.h>
#include "accelerator/apis.h"
#include "accelerator/config.h"
#include "accelerator/errors.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ta_http_s {
  bool running;
  void *daemon;
  ta_core_t *core;
} ta_http_t;

/**
 * Initialize logger
 */
void http_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int http_logger_release();

/**
 * Initializes an HTTP API
 *
 * @param http The HTTP status object
 * @param core An TA config information
 *
 * @return a status code
 */
status_t ta_http_init(ta_http_t *const http, ta_core_t *const core);

/**
 * Starts an HTTP API
 *
 * @param api The HTTP API
 *
 * @return a status code
 */
status_t ta_http_start(ta_http_t *const http);

/**
 * Stops an HTTP API
 *
 * @param api The HTTP API
 *
 * @return a status code
 */
status_t ta_http_stop(ta_http_t *const http);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_HTTP_H_
