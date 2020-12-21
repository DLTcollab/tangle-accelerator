/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "endpoint_core.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "build/endpoint_generated.h"
#include "common/logger.h"
#include "endpoint/cipher.h"
#include "endpoint/connectivity/conn_http.h"
#include "endpoint/https.h"
#include "endpoint/text_serializer.h"
#include "http_parser.h"
#include "legato.h"
#include "utils/tryte_byte_conv.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

#ifndef EP_TA_HOST
#define EP_TA_HOST localhost
#endif
#ifndef EP_TA_PORT
#define EP_TA_PORT 8000
#endif
#ifndef EP_SSL_SEED
#define EP_SSL_SEED nonce
#endif

#define REQ_BODY "{\"data\":{\"seed\":\"%s\",\"message\":\"%s\"},\"protocol\":\"MAM_V1\"}\r\n\r\n"

#define SEND_MAM_API "mam/send/"

#define ENDPOINT_LOGGER "endpoint"

static logger_id_t logger_id;

void endpoint_init() {
  ta_logger_init();
  // Logger initialization of endpoint
  logger_id = logger_helper_enable(ENDPOINT_LOGGER, LOGGER_DEBUG, true);

  // Logger initialization of other included components
  cipher_logger_init();
  https_logger_init();
}

void endpoint_destroy() {
  // Logger release of other included components
  cipher_logger_release();
  https_logger_release();

  // Logger release of endpoint
  logger_helper_release(logger_id);
  logger_helper_destroy();
}

status_t send_mam_message(const char* host, const char* port, const char* ssl_seed, const char* mam_seed,
                          const uint8_t* message, const size_t msg_len, const uint8_t* private_key,
                          const char* device_id) {
  int ret = 0;
  uint8_t* raw_msg = (uint8_t*)message;
  char req_body[MAX_MSG_LEN] = {0};
  uint8_t ciphertext[MAX_MSG_LEN] = {0};

  const char* ta_host = host;
  const char* ta_port = port;

  const char* seed = ssl_seed;

  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  ta_cipher_ctx encrypt_ctx = {.plaintext = raw_msg,
                               .plaintext_len = msg_len,
                               .ciphertext = ciphertext,
                               .ciphertext_len = MAX_MSG_LEN,
                               .device_id = device_id,
                               .iv = {0},
                               .hmac = {0},
                               .key = private_key,
                               .keybits = TA_AES_KEY_BITS,
                               .timestamp = t.tv_sec};
  ret = aes_encrypt(&encrypt_ctx);

  if (ret != SC_OK) {
    ta_log_error("Encrypt message error.\n");
    return ret;
  }

  flatcc_builder_t builder;
  flatcc_builder_init(&builder);

  flatbuffers_uint8_vec_ref_t hmac_vec;
  hmac_vec = flatbuffers_uint8_vec_create(&builder, encrypt_ctx.hmac, TA_AES_HMAC_SIZE);

  flatbuffers_uint8_vec_ref_t iv_vec;
  iv_vec = flatbuffers_uint8_vec_create(&builder, encrypt_ctx.iv, AES_IV_SIZE);

  flatbuffers_uint8_vec_ref_t msg_vec;
  msg_vec = flatbuffers_uint8_vec_create(&builder, encrypt_ctx.ciphertext, encrypt_ctx.ciphertext_len);

  FLATBUFFERS_WRAP_NAMESPACE(endpoint, Msg_create_as_root(&builder, hmac_vec, iv_vec, msg_vec));
  size_t buf_size = 0;
  uint8_t* buf = flatcc_builder_finalize_aligned_buffer(&builder, &buf_size);

  size_t encodedBuf_len = LE_BASE64_ENCODED_SIZE(buf_size) + 1;
  char* encodedBuf = malloc(encodedBuf_len);

  le_result_t result = le_base64_Encode(buf, buf_size, encodedBuf, &encodedBuf_len);
  if (LE_OK != result) {
    LE_ERROR("Error %d encoding data!", result);
    goto exit;
  }

  memset(req_body, 0, sizeof(char) * MAX_MSG_LEN);

  ret = snprintf(req_body, MAX_MSG_LEN, REQ_BODY, mam_seed, encodedBuf);
  free(encodedBuf);

  if (ret < 0) {
    ta_log_error("The message is too long.\n");
    return SC_ENDPOINT_SEND_TRANSFER;
  }

  https_response_t req = {
      .buffer = req_body,
      .len = ret,
  };

  https_response_t res = {0};
  https_ctx_t https_ctx = {
      .host = ta_host,
      .port = atoi(ta_port),
      .api = SEND_MAM_API,
      .ssl_seed = seed,
      .s_req = &req,
      .s_res = &res,
  };

  if (send_https_msg(&https_ctx) != SC_OK) {
    ta_log_error("http message sending error.\n");
    return SC_ENDPOINT_SEND_TRANSFER;
  }

  if (https_ctx.s_res->buffer != NULL) {
    ta_log_debug("HTTP Response: %s\n", https_ctx.s_res->buffer);
    free(https_ctx.s_res->buffer);
  }

exit:
  flatcc_builder_aligned_free(buf);
  flatcc_builder_clear(&builder);
  return SC_OK;
}
