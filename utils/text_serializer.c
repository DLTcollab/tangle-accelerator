/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "text_serializer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define IV_LEN 16
#define UINT32_LEN 10

status_t serialize_msg(const uint8_t *iv, uint32_t ciphertext_len, const char *ciphertext, char *out_msg,
                       size_t *out_msg_len) {
  /* FIXME: Provide some checks here */
  char str_ciphertext_len[UINT32_LEN + 1] = {0};
  char *ptr = out_msg;

  snprintf(str_ciphertext_len, UINT32_LEN + 1, "%010u", ciphertext_len);
  if (iv) {
    memcpy(ptr, iv, IV_LEN);
  } else {
    memset(ptr, 0, IV_LEN);
  }
  ptr += IV_LEN;

  memcpy(ptr, str_ciphertext_len, UINT32_LEN);
  ptr += UINT32_LEN;

  memcpy(ptr, ciphertext, ciphertext_len);

  *out_msg_len = IV_LEN + UINT32_LEN + ciphertext_len;

  return SC_OK;
}

status_t deserialize_msg(char *msg, const uint8_t *iv, size_t *ciphertext_len, char *ciphertext) {
  /* FIXME: Provide some checks here */
  char str_ciphertext_len[UINT32_LEN + 1] = {};
  char *ptr = msg;
  uint32_t ciphertext_len_tmp;

  memcpy((char *)iv, ptr, IV_LEN);
  ptr += IV_LEN;

  memcpy(str_ciphertext_len, ptr, UINT32_LEN);
  ciphertext_len_tmp = atoi(str_ciphertext_len);
  ptr += UINT32_LEN;

  memcpy(ciphertext, ptr, ciphertext_len_tmp);
  *ciphertext_len = ciphertext_len_tmp;

  return SC_OK;
}
