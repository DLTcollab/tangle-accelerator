#ifndef HTTP_HTTP_H_
#define HTTP_HTTP_H_

#include <stdbool.h>
#include "accelerator/core/apis.h"
#include "accelerator/core/proxy_apis.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file connectivity/http/http.h
 * @brief Router of HTTP protocol
 */

typedef struct ta_http_s {
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
 * @brief Starts an HTTP API
 *
 * @param http The HTTP API
 *
 * @return a status code
 */
status_t ta_http_start(ta_http_t *const http);

/**
 * @brief Stops an HTTP API
 *
 * @param http The HTTP API
 *
 * @return a status code
 */
status_t ta_http_stop(ta_http_t *const http);

#ifdef __cplusplus
}
#endif

#endif  // HTTP_HTTP_H_
