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
#include "common/ta_errors.h"
#include "endpoint/cipher.h"
#include "endpoint/endpoint_core.h"
#include "tests/test_define.h"

#define TEST_VALUE 0
#define TEST_MESSAGE "THISISMSG9THISISMSG9THISISMSG"
#define TEST_MAM_SEED                                                          \
  "POWEREDBYTANGLEACCELERATOR999999999999999999999999999999999999999999999999" \
  "999999A"
#define TEST_DEVICE_ID "470010171566423"

static char* TEST_TA_HOST;
static char* TEST_TA_PORT;

const uint8_t test_key[32] = {82,  142, 184, 64,  74, 105, 126, 65,  154, 116, 14,  193, 208, 41,  8,  115,
                              158, 252, 228, 160, 79, 5,   167, 185, 13,  159, 135, 113, 49,  209, 58, 68};
const uint8_t test_iv[AES_IV_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};

void gen_rand_trytes(uint8_t* out, size_t len) {
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
  struct tm* tm_info;

  time(&timer);
  tm_info = localtime(&timer);
  strftime(time_str, 26, "%Y-%m-%d %H:%M:%S", tm_info);
  gen_rand_trytes(next_addr, ADDR_LEN);

  TEST_ASSERT(SC_OK == send_mam_message(TEST_TA_HOST, TEST_TA_PORT, NULL, TEST_MAM_SEED, TEST_MESSAGE, test_key,
                                        TEST_DEVICE_ID, iv));
}

int main(int argc, char** argv) {
  UNITY_BEGIN();
  if (argc != 3) {
    printf("usage: ./test_endpoint_core [TA_HOST] [TA_PORT]");
    return UNITY_END();
  }

  TEST_TA_HOST = argv[1];
  TEST_TA_PORT = argv[2];

  endpoint_init();

  RUN_TEST(test_endpoint);

  endpoint_destroy();
  return UNITY_END();
}
