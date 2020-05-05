/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "common/macros.h"
#include "common/ta_errors.h"
#include "endpoint.h"
#include "tests/test_define.h"
#include "utils/cipher.h"

#define TEST_VALUE 0
#define TEST_MESSAGE "THISISMSG9THISISMSG9THISISMSG"
#define TEST_MESSAGE_FMT "ascii"
#define TEST_DEVICE_ID "470010171566423"

#define TEST_REQ_BODY                               \
  "{\"value\":" TEST_VALUE ", \"tag\": \"" TEST_TAG \
  "\", \"message\": "                               \
  "\"" TEST_MESSAGE "\", \"address\": \"" TEST_ADDRESS "\"}\r\n\r\n"

const uint8_t test_key[32] = {82,  142, 184, 64,  74, 105, 126, 65,  154, 116, 14,  193, 208, 41,  8,  115,
                              158, 252, 228, 160, 79, 5,   167, 185, 13,  159, 135, 113, 49,  209, 58, 68};
const uint8_t test_iv[AES_IV_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};

void gen_rand_trytes(uint8_t *out, size_t len) {
  const char tryte_alphabet[] = "9ABCDEFGHIJKLMNOPQRSTUVWXYZ";
  const int alphabet_array_length = ARRAY_SIZE(tryte_alphabet);
  for (size_t i = 0; i < len; i++) {
    uint8_t rand_index = rand() % alphabet_array_length;
    out[i] = tryte_alphabet[rand_index];
  }
}

void test_endpoint(void) {
  uint8_t next_addr[ADDR_LEN] = {0};
  uint8_t iv[AES_IV_SIZE] = {0};

  memcpy(iv, test_iv, AES_IV_SIZE);
  srand(time(NULL));

  time_t timer;
  char time_str[26];
  struct tm *tm_info;

  time(&timer);
  tm_info = localtime(&timer);
  strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  gen_rand_trytes(next_addr, ADDR_LEN);

  status_t ret = send_transaction_information(TEST_VALUE, TEST_MESSAGE, TEST_MESSAGE_FMT, TEST_TAG, TEST_ADDRESS,
                                              (char *)next_addr, test_key, TEST_DEVICE_ID, iv);
  TEST_ASSERT(ret == SC_OK);
}

int main(void) {
  UNITY_BEGIN();
  RUN_TEST(test_endpoint);
  return UNITY_END();
}
