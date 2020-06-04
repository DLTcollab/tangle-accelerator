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
#include "http_parser.h"
#include "utils/cipher.h"
#include "utils/connectivity/conn_http.h"
#include "utils/https.h"
#include "utils/text_serializer.h"
#include "utils/tryte_byte_conv.h"

// FIXME: Same as STR() inside tests/test_defined.h
#define STR_HELPER(num) #num
#define STR(num) STR_HELPER(num)

static const char* HOST = STR(ENDPOINT_HOST);
static const char* PORT = STR(ENDPOINT_PORT);
static const char* SSL_SEED = STR(ENDPOINT_SSL_SEED);

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
}

void endpoint_destroy() {
  // Logger release of other included components
  cipher_logger_release();

  // Logger release of endpoint
  logger_helper_release(logger_id);
  logger_helper_destroy();
}

status_t send_transaction_information(int value, const char* message, const char* message_fmt,
                                      const char* tag, const char* address, const char* next_address,
                                      const uint8_t* private_key, const char* device_id, uint8_t* iv) {
  char tryte_msg[MAX_MSG_LEN] = {0};
  char msg[MAX_MSG_LEN] = {0};
  char req_body[MAX_MSG_LEN] = {0};
  uint8_t ciphertext[MAX_MSG_LEN] = {0};
  uint8_t raw_msg[MAX_MSG_LEN] = {0};

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

  if (send_https_msg(HOST, PORT, SEND_TRANSACTION_API, req_body, SSL_SEED) != SC_OK) {
    ta_log_error("http message sending error.\n");
    return SC_ENDPOINT_SEND_TRANSFER;
  }

  return SC_OK;
}