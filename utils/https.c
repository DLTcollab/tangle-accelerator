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
#include "common/defined_error.h"
#include "http_parser.h"
#include "utils/connectivity/conn_http.h"

static http_parser parser;

endpoint_retcode_t send_https_msg(char const *host, char const *port, char const *api, const char *msg,
                                  const int msg_len, const char *ssl_seed) {
  char res[4096] = {0};
  char *req = NULL;
  endpoint_retcode_t ret = RET_OK;

  set_post_request(api, host, atoi(port), msg, &req);
  http_parser_settings settings;
  settings.on_body = parser_body_callback;

  connect_info_t info = {.https = true};
  /* FIXME:Provide some checks here */
  http_open(&info, ssl_seed, host, port);
  http_send_request(&info, req);
  http_read_response(&info, res, sizeof(res) / sizeof(char));
  http_close(&info);
  http_parser_init(&parser, HTTP_RESPONSE);
  http_parser_execute(&parser, &settings, res, strlen(res));

  if (parser.status_code != HTTP_OK) {
    ret = RET_FAULT;
  }

  free(req);
  return ret;
}