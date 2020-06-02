/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "text_serializer.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "utils/cipher.h"

#define IV_LEN 16
#define UINT32_LEN 10
#define UINT64_LEN 20
#define STR_HELPER(num) #num
#define STR(num) STR_HELPER(num)

status_t serialize_msg(const ta_cipher_ctx *ctx, char *out_msg, size_t *out_msg_len) {
  /* FIXME: Provide some checks here */
  char str_ciphertext_len[UINT32_LEN + 1] = {0};
  char buf[UINT64_LEN + 1] = {0};
  char *ptr = out_msg;

  if (out_msg == NULL || out_msg_len == NULL) {
    // FIXME: Use default logger
    fprintf(stderr, "The output message and output message length cannot be NULL\n");
    return SC_UTILS_TEXT_SERIALIZE;
  }

  if (ctx->ciphertext == NULL) {
    // FIXME: Use default logger
    fprintf(stderr, "The ciphertext cannot be NULL\n");
    return SC_UTILS_TEXT_SERIALIZE;
  }

  snprintf(str_ciphertext_len, UINT32_LEN + 1, "%0" STR(UINT32_LEN) "zu", ctx->ciphertext_len);
  // initialize vector
  memcpy(ptr, ctx->iv, IV_LEN);
  ptr += IV_LEN;
  // timestamp
  snprintf(buf, UINT64_LEN + 1, "%020" PRIu64, ctx->timestamp);
  memcpy(ptr, buf, UINT64_LEN);
  ptr += UINT64_LEN;
  // hmac
  memcpy(ptr, ctx->hmac, TA_AES_HMAC_SIZE);
  ptr += TA_AES_HMAC_SIZE;
  // ciphertext length
  memcpy(ptr, str_ciphertext_len, UINT32_LEN);
  ptr += UINT32_LEN;
  // ciphertext
  memcpy(ptr, ctx->ciphertext, ctx->ciphertext_len);
  *out_msg_len = IV_LEN + UINT64_LEN + TA_AES_HMAC_SIZE + UINT32_LEN + ctx->ciphertext_len;

  return SC_OK;
}

status_t deserialize_msg(const char *msg, ta_cipher_ctx *ctx) {
  /* FIXME: Provide some checks here */
  char str_ciphertext_len[UINT32_LEN + 1] = {};
  char buf[UINT64_LEN + 1] = {0};
  const char *ptr = msg;
  if (ptr == NULL) {
    // FIXME: Use default logger
    fprintf(stderr, "The message cannot be NULL\n");
    return SC_UTILS_TEXT_DESERIALIZE;
  }

  if (ctx->ciphertext == NULL) {
    // FIXME: Use default logger
    fprintf(stderr, "The ciphertext cannot be NULL\n");
    return SC_UTILS_TEXT_DESERIALIZE;
  }
  uint32_t ciphertext_len_tmp;
  // initialize vector
  memcpy(ctx->iv, ptr, IV_LEN);
  ptr += IV_LEN;
  // timestamp
  memcpy(buf, ptr, UINT64_LEN);
  ctx->timestamp = atol(buf);
  ptr += UINT64_LEN;
  // hmac
  memcpy(ctx->hmac, ptr, TA_AES_HMAC_SIZE);
  ptr += TA_AES_HMAC_SIZE;
  // ciphertext length
  memcpy(str_ciphertext_len, ptr, UINT32_LEN);
  ciphertext_len_tmp = atoi(str_ciphertext_len);
  ptr += UINT32_LEN;
  // ciphertext
  memcpy(ctx->ciphertext, ptr, ciphertext_len_tmp);
  ctx->ciphertext_len = ciphertext_len_tmp;

  return SC_OK;
}
