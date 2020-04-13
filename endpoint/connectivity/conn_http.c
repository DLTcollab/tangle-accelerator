/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <errno.h>
#include <netdb.h>
#include <signal.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include "ca_cert.h"
#include "conn_http.h"

#define MAX_PORT_LEN 5
#define MAX_CONTENT_LENGTH_LEN 5

void mbedtls_debug(void *ctx, int level, const char *file, int line, const char *str) {
  ((void)level);
  fprintf((FILE *)ctx, "%s:%04d: %s", file, line, str);
  fflush((FILE *)ctx);
}

retcode_t http_open(connect_info_t *const info, char const *const seed_nonce, char const *const host,
                    char const *const port) {
  int ret;

  if (info->https) {
    info->net_ctx = (mbedtls_net_context *)malloc(sizeof(mbedtls_net_context));
    info->entropy = (mbedtls_entropy_context *)malloc(sizeof(mbedtls_entropy_context));
    info->ctr_drbg = (mbedtls_ctr_drbg_context *)malloc(sizeof(mbedtls_ctr_drbg_context));
    info->ssl_ctx = (mbedtls_ssl_context *)malloc(sizeof(mbedtls_ssl_context));
    info->ssl_config = (mbedtls_ssl_config *)malloc(sizeof(mbedtls_ssl_config));
    info->cacert = (mbedtls_x509_crt *)malloc(sizeof(mbedtls_x509_crt));

    mbedtls_net_init(info->net_ctx);
    mbedtls_ssl_init(info->ssl_ctx);
    mbedtls_ssl_config_init(info->ssl_config);
    mbedtls_x509_crt_init(info->cacert);
    mbedtls_ctr_drbg_init(info->ctr_drbg);
    mbedtls_entropy_init(info->entropy);

    ret = mbedtls_ctr_drbg_seed(info->ctr_drbg, mbedtls_entropy_func, info->entropy, (const unsigned char *)seed_nonce,
                                strlen(seed_nonce));
    if (ret != 0) {
      free(info->net_ctx);
      free(info->ssl_ctx);
      free(info->ssl_config);
      free(info->ctr_drbg);
      free(info->entropy);
      free(info->cacert);
      return RET_HTTP_INIT;
    }

    // TODO Change the PEM file path
    ret = mbedtls_x509_crt_parse(info->cacert, (const unsigned char *)mbedtls_test_cas_pem, mbedtls_test_cas_pem_len);
    if (ret < 0) {
      printf("error: mbedtls_x509_crt_parse returned -0x%x\n\n", -ret);
      return RET_HTTP_CERT;
    }
  }

  ret = mbedtls_net_connect(info->net_ctx, host, port, MBEDTLS_NET_PROTO_TCP);
  if (ret != 0) {
    printf("error: mbedtls_net_connect returned %d\n\n", ret);
    return RET_HTTP_CONNECT;
  }

  ret = mbedtls_ssl_config_defaults(info->ssl_config, MBEDTLS_SSL_IS_CLIENT, MBEDTLS_SSL_TRANSPORT_STREAM,
                                    MBEDTLS_SSL_PRESET_DEFAULT);
  if (ret != 0) {
    printf("error: mbedtls_ssl_config_defaults returned %d\n\n", ret);
    return RET_HTTP_SSL;
  }

  // TODO we may need to set the mode into MBEDTLS_SSL_VERIFY_REQUIRED
  mbedtls_ssl_conf_authmode(info->ssl_config, MBEDTLS_SSL_VERIFY_OPTIONAL);

  mbedtls_ssl_conf_rng(info->ssl_config, mbedtls_ctr_drbg_random, info->ctr_drbg);
  mbedtls_ssl_conf_dbg(info->ssl_config, mbedtls_debug, stdout);

  ret = mbedtls_ssl_setup(info->ssl_ctx, info->ssl_config);
  if (ret != 0) {
    printf("error: mbedtls_ssl_setup returned %d\n\n", ret);
    return RET_HTTP_SSL;
  }

  ret = mbedtls_ssl_set_hostname(info->ssl_ctx, host);
  if (ret != 0) {
    printf("error: mbedtls_ssl_set_hostname returned %d\n\n", ret);
    return RET_HTTP_SSL;
  }

  // Here is Blocking mode
  mbedtls_ssl_set_bio(info->ssl_ctx, info->net_ctx, mbedtls_net_send, mbedtls_net_recv, mbedtls_net_recv_timeout);

  ret = mbedtls_ssl_handshake(info->ssl_ctx);
  while (ret != 0) {
    if (ret != MBEDTLS_ERR_SSL_WANT_READ && ret != MBEDTLS_ERR_SSL_WANT_WRITE) {
      printf("error: mbedtls_ssl_handshake returned -0x%x\n\n", -ret);
      return RET_HTTP_SSL;
    }
  }

#if 0
  // Verify the server X.509 certificate
  /* In real life, we probably want to bail out when ret != 0 */
  uint32_t flags = mbedtls_ssl_get_verify_result(info->ssl_ctx);
  if (flags != 0) {
    char vrfy_buf[512];
    printf("error: Verifying peer X.509 certificate failed\n");
    mbedtls_x509_crt_verify_info(vrfy_buf, sizeof(vrfy_buf), "", flags);
    printf("error: %s\n", vrfy_buf);
  }
#endif
  return RET_OK;
}

retcode_t http_send_request(connect_info_t *const info, const char *req) {
  size_t req_len = strlen(req), write_len = 0, ret_len;

  while (write_len < req_len) {
    if (info->https) {
      ret_len = mbedtls_ssl_write(info->ssl_ctx, (unsigned char *)(req + write_len), (req_len - write_len));
    } else {
      ret_len = mbedtls_net_send(info->ssl_ctx, (unsigned char *)(req + write_len), (req_len - write_len));
    }

    if (ret_len == MBEDTLS_ERR_SSL_WANT_WRITE) {
      continue;
    } else if (ret_len <= 0) {
      return RET_WRITE_ERROR;
    }
    write_len += ret_len;
  }
  return RET_OK;
}

retcode_t http_read_response(connect_info_t *const info, char *res, size_t res_len) {
  size_t ret_len;
  const uint32_t timeout_period = 5000;  // in milliseconds

  if (info->https) {
    ret_len = mbedtls_ssl_read(info->ssl_ctx, (unsigned char *)res, res_len);
  } else {
    ret_len = mbedtls_net_recv_timeout(info->net_ctx, (unsigned char *)res, res_len, timeout_period);
  }

  switch (ret_len) {
    case 0:
    case MBEDTLS_ERR_SSL_WANT_READ:
    case MBEDTLS_ERR_SSL_WANT_WRITE:
    case MBEDTLS_ERR_SSL_ASYNC_IN_PROGRESS:
    case MBEDTLS_ERR_SSL_CRYPTO_IN_PROGRESS:
      mbedtls_ssl_session_reset(info->ssl_ctx);
      break;

    default:
      break;
  }
  return RET_OK;
}

retcode_t http_close(connect_info_t *const info) {
  if (info->https) {
    mbedtls_ssl_close_notify(info->ssl_ctx);

    mbedtls_net_free(info->net_ctx);
    mbedtls_entropy_free(info->entropy);
    mbedtls_ctr_drbg_free(info->ctr_drbg);
    mbedtls_ssl_free(info->ssl_ctx);
    mbedtls_ssl_config_free(info->ssl_config);
    mbedtls_x509_crt_free(info->cacert);
    free(info->net_ctx);
    free(info->ssl_ctx);
    free(info->ssl_config);
    free(info->ctr_drbg);
    free(info->entropy);
    free(info->cacert);
  } else {
    mbedtls_net_free(info->net_ctx);
    free(info->net_ctx);
  }

  return RET_OK;
}

retcode_t set_post_request(char const *const path, char const *const host, const uint32_t port,
                           char const *const req_body, char **out) {
  const char post_req_format[] =
      "POST /%s HTTP/1.1\r\n"
      "Host: %s:%d\r\n"
      "Connection: Keep-Alive\r\n"
      "Content-Type: application/json\r\n"
      "Content-Length: %d\r\n"
      "\r\n"
      "%s\r\n\r\n";

  size_t out_len =
      strlen(post_req_format) + strlen(path) + strlen(host) + MAX_PORT_LEN + MAX_CONTENT_LENGTH_LEN + strlen(req_body);
  *out = (char *)malloc(sizeof(char) * out_len);
  if (!*out) {
    return RET_OOM;
  }
  snprintf(*out, out_len, post_req_format, path, host, port, (int)strlen(req_body), req_body);

  return RET_OK;
}

retcode_t set_get_request(char const *const path, char const *const host, const uint32_t port, char **out) {
  const char get_req_format[] =
      "GET /%s HTTP/1.1\r\n"
      "Host: %s:%d\r\n"
      "\r\n\r\n";

  size_t out_len = strlen(get_req_format) + strlen(path) + strlen(host) + MAX_PORT_LEN;
  *out = (char *)malloc(sizeof(char) * out_len);
  if (!*out) {
    return RET_OOM;
  }
  snprintf(*out, out_len, get_req_format, path, host, port);

  return RET_OK;
}

int parser_body_callback(http_parser *parser, const char *at, size_t length) {
#ifdef DEBUG
  printf("HTTP Response: %s\n", at);
#endif
  return 0;
}
