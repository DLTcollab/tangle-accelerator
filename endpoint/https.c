/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "https.h"
#include <stdlib.h>
#include <string.h>
#include "common/logger.h"
#include "common/ta_errors.h"
#include "endpoint/connectivity/conn_http.h"
#include "http_parser.h"

static http_parser parser;

#define HTTPS_LOGGER "https"
static logger_id_t logger_id;

void https_logger_init() { logger_id = logger_helper_enable(HTTPS_LOGGER, LOGGER_DEBUG, true); }

int https_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", HTTPS_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

status_t send_https_msg(char const *host, char const *port, char const *api, const char *msg, const char *ssl_seed) {
  char res[4096] = {0};
  char *req = NULL;
  status_t ret = SC_OK;

  set_post_request(api, host, atoi(port), msg, &req);
  http_parser_settings settings = {};
  settings.on_body = parser_body_callback;

#ifdef ENDPOINT_HTTPS
  connect_info_t info = {.https = true};
#else
  connect_info_t info = {.https = false};
#endif

  ret = http_open(&info, ssl_seed, host, port);
  if (ret != SC_OK) {
    ta_log_error("http(s) open error, return code %d\n", ret);
    return ret;
  }

  ret = http_send_request(&info, req);
  if (ret != SC_OK) {
    ta_log_error("http(s) send request error, return code %d\n", ret);
    goto exit;
  }

  ret = http_read_response(&info, res, sizeof(res) / sizeof(char));
  if (ret != SC_OK) {
    ta_log_error("http(s) read response error, return code %d\n", ret);
    goto exit;
  }

  http_parser_init(&parser, HTTP_RESPONSE);
  http_parser_execute(&parser, &settings, res, strlen(res));

  if (parser.status_code != SC_HTTP_OK) {
    ta_log_error("http(s): fail to parse response\n");
    ret = SC_UTILS_HTTPS_RESPONSE_ERROR;
  }

exit:
  http_close(&info);
  free(req);
  return ret;
}
