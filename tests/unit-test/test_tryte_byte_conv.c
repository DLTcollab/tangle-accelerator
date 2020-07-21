/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "tests/test_define.h"
#include "utils/tryte_byte_conv.h"

void setUp(void) {}

void tearDown(void) {}

void test_bytes_trytes_bytes_conv(void) {
  const uint8_t test_str[1024] = {48, 48, 48, 48, 48, 0,  48, 48, 48, 48, 48, 48, 48, 48,
                                  48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48, 48};
  const int str_len = 28;
  char enc_msg[1024] = {0}, dec_msg[1024] = {0};

  bytes_to_trytes(test_str, str_len, enc_msg);
  trytes_to_bytes((uint8_t*)enc_msg, strlen(enc_msg), dec_msg);

  TEST_ASSERT_EQUAL_INT8_ARRAY(test_str, dec_msg, str_len);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_bytes_trytes_bytes_conv);

  return UNITY_END();
}
