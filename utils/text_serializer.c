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

status_t serialize_msg(const uint8_t *iv, uint32_t ciphertext_len, const char *ciphertext, const uint64_t timestamp,
                       const uint8_t *hmac, char *out_msg, size_t *out_msg_len) {
  /* FIXME: Provide some checks here */
  char str_ciphertext_len[UINT32_LEN + 1] = {0};
  char buf[UINT64_LEN + 1] = {0};
  char *ptr = out_msg;

  snprintf(str_ciphertext_len, UINT32_LEN + 1, "%010u", ciphertext_len);
  // initialize vector
  if (iv) {
    memcpy(ptr, iv, IV_LEN);
  } else {
    memset(ptr, 0, IV_LEN);
  }
  ptr += IV_LEN;

  // timestamp
  snprintf(buf, UINT64_LEN + 1, "%020" PRIu64, timestamp);
  memcpy(ptr, buf, UINT64_LEN);
  ptr += UINT64_LEN;
  // hmac
  memcpy(ptr, hmac, TA_AES_HMAC_SIZE);
  ptr += TA_AES_HMAC_SIZE;
  // ciphertext length
  memcpy(ptr, str_ciphertext_len, UINT32_LEN);
  ptr += UINT32_LEN;
  // ciphertext
  memcpy(ptr, ciphertext, ciphertext_len);
  *out_msg_len = IV_LEN + UINT64_LEN + TA_AES_HMAC_SIZE + UINT32_LEN + ciphertext_len;

  return SC_OK;
}

status_t deserialize_msg(char *msg, const uint8_t *iv, size_t *ciphertext_len, char *ciphertext, uint64_t *timestamp,
                         uint8_t *hmac) {
  /* FIXME: Provide some checks here */
  char str_ciphertext_len[UINT32_LEN + 1] = {};
  char buf[UINT64_LEN + 1] = {0};
  char *ptr = msg;
  uint32_t ciphertext_len_tmp;
  // initialize vector
  memcpy((char *)iv, ptr, IV_LEN);
  ptr += IV_LEN;
  // timestamp
  memcpy(buf, ptr, UINT64_LEN);
  *timestamp = atol(buf);
  ptr += UINT64_LEN;
  // hmac
  memcpy(hmac, ptr, TA_AES_HMAC_SIZE);
  ptr += TA_AES_HMAC_SIZE;
  // ciphertext length
  memcpy(str_ciphertext_len, ptr, UINT32_LEN);
  ciphertext_len_tmp = atoi(str_ciphertext_len);
  ptr += UINT32_LEN;
  // ciphertext
  memcpy(ciphertext, ptr, ciphertext_len_tmp);
  *ciphertext_len = ciphertext_len_tmp;

  return SC_OK;
}
