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
  return 0;
}

static int https_parser_callback(http_parser* parser, const char* at, size_t length) {
  if (parser == NULL || parser->data == NULL) {
    ta_log_error("http(s) parser: parser or parser->data cannot be NULL");
    /* Returning a non-zero value indicates error to the parser */
    return 1;
  }
  https_response_t* my_data = (https_response_t*)parser->data;

  my_data->buffer = malloc(sizeof(char) * (length + 1));
  if (my_data->buffer == NULL) {
    ta_log_error("http(s) parser: cannot allocate enough memory");
    /* Returning a non-zero value indicates error to the parser */
    return 1;
  }
  snprintf(my_data->buffer, length + 1, "%s", at);
  my_data->len = length;
  return 0;
}

status_t send_https_msg(https_ctx_t* ctx) {
  char res[4096] = {0};
  char* req = NULL;
  status_t ret = SC_OK;

  const char* host = ctx->host;
  const char* api = ctx->api;
  const int port = ctx->port;
  const char* ssl_seed = ctx->ssl_seed;
  const char* msg = ctx->s_req->buffer;

  set_post_request(api, host, ctx->port, msg, &req);

  http_parser_settings settings = {};
  settings.on_body = https_parser_callback;

  https_response_t my_data = {0};
  parser.data = &my_data;

#ifdef ENDPOINT_HTTPS
  connect_info_t info = {.https = true};
#else
  connect_info_t info = {.https = false};
#endif

  char net_port[12] = {0};
  snprintf(net_port, 12, "%d", port);
  ret = http_open(&info, ssl_seed, host, net_port);
  if (ret != SC_OK) {
    ta_log_error("http(s) open error, return code %d\n", ret);
    goto exit;
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

  ctx->s_res->buffer = my_data.buffer;
  ctx->s_res->len = my_data.len;

exit:
  http_close(&info);
  free(req);
  return ret;
}
