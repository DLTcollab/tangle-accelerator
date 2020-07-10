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
#include "common/logger.h"
#include "endpoint/cipher.h"
#include "endpoint/connectivity/conn_http.h"
#include "endpoint/https.h"
#include "endpoint/text_serializer.h"
#include "http_parser.h"
#include "utils/tryte_byte_conv.h"

#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>

// FIXME: Same as STR() inside tests/test_defined.h
#define STR_HELPER(num) #num
#define STR(num) STR_HELPER(num)

#ifndef EP_TA_HOST
#define EP_TA_HOST localhost
#endif
#ifndef EP_TA_PORT
#define EP_TA_PORT 8000
#endif
#ifndef EP_SSL_SEED
#define EP_SSL_SEED nonce
#endif

#define REQ_BODY \
  "{\"value\": %d, \"message\": \"%s\", \"message_format\": \"%s\", \"tag\": \"%s\", \"address\": \"%s\"}\r\n\r\n"

#define SEND_TRANSACTION_API "transaction/"

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

status_t send_transaction_information(const char* host, const char* port, const char* ssl_seed, int value,
                                      const char* message, const char* message_fmt, const char* tag,
                                      const char* address, const char* next_address, const uint8_t* private_key,
                                      const char* device_id, uint8_t* iv) {
  char tryte_msg[MAX_MSG_LEN] = {0};
  char msg[MAX_MSG_LEN] = {0};
  char req_body[MAX_MSG_LEN] = {0};
  uint8_t ciphertext[MAX_MSG_LEN] = {0};
  uint8_t raw_msg[MAX_MSG_LEN] = {0};

  const char* ta_host = host ? host : STR(EP_TA_HOST);
  const char* ta_port = port ? port : STR(EP_TA_PORT);

  const char* seed = ssl_seed ? ssl_seed : STR(EP_SSL_SEED);

  int ret = snprintf((char*)raw_msg, MAX_MSG_LEN, "%s:%s", next_address, message);
  if (ret < 0) {
    ta_log_error("The message is too long.\n");
    return SC_ENDPOINT_SEND_TRANSFER;
  }

  size_t msg_len = 0;
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  ta_cipher_ctx encrypt_ctx = {.plaintext = raw_msg,
                               .plaintext_len = ret,
                               .ciphertext = ciphertext,
                               .ciphertext_len = MAX_MSG_LEN,
                               .device_id = device_id,
                               .iv = {0},
                               .hmac = {0},
                               .key = private_key,
                               .keybits = TA_AES_KEY_BITS,
                               .timestamp = t.tv_sec};
  memcpy(encrypt_ctx.iv, iv, AES_IV_SIZE);
  ret = aes_encrypt(&encrypt_ctx);
  memcpy(iv, encrypt_ctx.iv, AES_IV_SIZE);

  if (ret != SC_OK) {
    ta_log_error("Encrypt message error.\n");
    return ret;
  }
  serialize_msg(&encrypt_ctx, msg, &msg_len);
  bytes_to_trytes((const unsigned char*)msg, msg_len, tryte_msg);

  memset(req_body, 0, sizeof(char) * MAX_MSG_LEN);

  ret = snprintf(req_body, MAX_MSG_LEN, REQ_BODY, value, tryte_msg, message_fmt, tag, address);
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
      .api = SEND_TRANSACTION_API,
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

  return SC_OK;
}
