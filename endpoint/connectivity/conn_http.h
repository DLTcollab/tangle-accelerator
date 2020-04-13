/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CONN_HTTP_H
#define CONN_HTTP_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "defined_error.h"
#include "http_parser.h"
#include "mbedtls/certs.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/entropy.h"
#include "mbedtls/error.h"
#include "mbedtls/net.h"
#include "mbedtls/ssl.h"

typedef struct {
  bool https;                         /**< Flag for check info has initialized or not */
  mbedtls_net_context *net_ctx;       /**< mbedtls_next context                       */
  mbedtls_entropy_context *entropy;   /**< mbedtls_entropy context                    */
  mbedtls_ctr_drbg_context *ctr_drbg; /**< mbedlts_ctr_drbg context                   */
  mbedtls_ssl_context *ssl_ctx;       /**< mbedtls_ssl context                        */
  mbedtls_ssl_config *ssl_config;     /**< mbedtls_ssl_config context                 */
  mbedtls_x509_crt *cacert;           /**< mbedtls_x509 container                     */
} connect_info_t;

/**
 * @brief Open HTTP connection
 *
 * @param[in, out] info Context for HTTP connection
 * @param[in] seed_nonce Personalization data, that is device-specific identifiers. Can be NULL.
 * @param[in] host HTTP host to connect
 * @param[in] port HTTP port to connect
 *
 * @return
 * - #RET_HTTP_INIT failed on HTTP init error
 * - #RET_HTTP_CONNECT failed on HTTP connect error
 * - #RET_HTTP_CERT failed on HTTP certificate setting error
 * - #RET_HTTP_SSL failed on HTTP ssl setting error
 * - #RET_OK on success
 * @see #retcode_t
 */
retcode_t http_open(connect_info_t *const info, char const *const seed_nonce, char const *const host,
                    char const *const port);

/**
 * @brief Send request to HTTP connection
 *
 * @param[in] info Context for HTTP connection
 * @param[in] req Buffer holding the data
 *
 * @return
 * - #RET_OK on success
 * - non-zero on error
 * @see #retcode_t
 */
retcode_t http_send_request(connect_info_t *const info, const char *req);

/**
 * @brief Read response from HTTP server
 *
 * @param[in] info Context for HTTP connection
 * @param[out] res Buffer that will hold the data
 * @param[out] res_len Length of res
 *
 * @return
 * - #RET_OK on success
 * - non-zero on error
 * @see #retcode_t
 */
retcode_t http_read_response(connect_info_t *const info, char *res, size_t res_len);

/**
 * @brief Close HTTP connection
 *
 * @param[in] info Context for HTTP connection
 *
 * @return
 * - #RET_OK on success
 * - non-zero on error
 * @see #retcode_t
 */
retcode_t http_close(connect_info_t *const info);

/**
 * @brief Set POST request message
 *
 * @param[in] path API path for POST request to HTTP(S) server, i.e "transaction/". It
 * must be in string.
 * @param[in] host HTTP host to connect
 * @param[in] port HTTP port to connect
 * @param[in] req_body Pointer of POST request body
 * @param[out] out POST request message
 *
 * @return
 * - #RET_OK on success
 * - #RET_OOM failed on out of memory error
 * - non-zero on error
 * @see #retcode_t
 */
retcode_t set_post_request(char const *const path, char const *const host, const uint32_t port,
                           char const *const req_body, char **out);
/**
 * @brief Set GET request message
 *
 * @param[in] path API path for GET request to HTTP(S) server, i.e "transaction/". It
 * must be in string.
 * @param[in] host HTTP host to connect
 * @param[in] port HTTP port to connect
 * @param[out] out GET request message
 *
 * @return
 * - #RET_OK on success
 * - #RET_OOM failed on out of memory error
 * - non-zero on error
 * @see #retcode_t
 */
retcode_t set_get_request(char const *const path, char const *const host, const uint32_t port, char **out);

/**
 * @brief Callback function for http parser
 *
 * @param[in] parser HTTP parser
 * @param[in] at HTTP Message to parse
 * @param[in] length Length of text at
 *
 * @return
 * - 0 on success
 * - non-zero on error
 */
int parser_body_callback(http_parser *parser, const char *at, size_t length);

#ifdef __cplusplus
}
#endif

#endif  // CONN_HTTP_H
