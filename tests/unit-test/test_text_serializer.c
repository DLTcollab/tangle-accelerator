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
#include "utils/cipher.h"
#include "utils/text_serializer.h"

const uint8_t iv[AES_BLOCK_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};
const uint8_t payload[] = {
    99,  44,  121, 217, 149, 161, 127, 33,  133, 77,  125, 156, 53,  53,  248, 95,  57,  196, 141, 90,  121, 158,
    133, 218, 153, 153, 24,  84,  32,  245, 68,  131, 33,  189, 93,  182, 94,  220, 215, 227, 42,  85,  127, 95,
    138, 119, 190, 196, 60,  75,  30,  181, 233, 164, 143, 130, 61,  167, 214, 93,  156, 26,  225, 189, 216, 62,
    116, 54,  26,  75,  26,  68,  160, 153, 163, 43,  17,  97,  239, 77,  172, 13,  0,   149, 177, 145, 24,  239,
    57,  238, 76,  213, 9,   45,  147, 225, 107, 7,   23,  134, 82,  49,  202, 243, 203, 110, 30,  220, 207, 13,
    41,  124, 26,  43,  17,  204, 188, 41,  187, 245, 24,  7,   203, 33,  53,  94,  2,   160, 101, 25,  38,  183,
    75,  241, 170, 22,  95,  200, 242, 46,  213, 27,  170, 240, 70,  188, 188, 2,   229, 119, 248, 253, 126, 195,
    30,  179, 33,  32,  84,  134, 58,  122, 61,  133, 107, 232, 155, 202, 176, 141, 249, 134, 168, 163, 118, 238,
    95,  50,  240, 69,  169, 232, 66,  39,  171, 97,  219, 204, 129, 47,  82,  187, 169, 144, 64,  21,  120, 219,
    223, 40,  104, 216, 174, 16,  124, 36,  254, 219, 86,  239, 32,  255, 215, 99,  39,  131, 196, 2,   79,  69,
    49,  162, 1,   218, 50,  65,  239, 170, 29,  207, 210, 133, 167, 129, 150, 35,  165, 148, 255, 252, 131, 31,
    251, 91,  130, 34,  222, 70,  36,  45,  140, 85,  207, 141, 48,  1,   206, 31,  171, 235, 238, 126, 113};
const uint16_t payload_len = 263;
const uint64_t test_timestamp = 1589274915;
const uint8_t test_hmac[TA_AES_HMAC_SIZE] = {37, 39, 48, 30, 53, 63, 45, 61, 87, 60, 60, 90, 63, 63, 13, 94,
                                             53, 93, 52, 7,  19, 38, 15, 31, 29, 32, 97, 2,  47, 67, 59, 62};

void setUp(void) {}

void tearDown(void) {}

void test_serialize_deserialize(void) {
  uint8_t out[1024], payload_out[1024];
  size_t out_msg_len = 0;
  ta_cipher_ctx serialize_ctx = {
      .ciphertext = (uint8_t*)payload,
      .ciphertext_len = payload_len,
      .timestamp = test_timestamp,
      .hmac = {0},
      .iv = {0},
  };
  memcpy(serialize_ctx.iv, iv, AES_IV_SIZE);
  memcpy(serialize_ctx.hmac, test_hmac, TA_AES_HMAC_SIZE);

  int rc1 = serialize_msg(&serialize_ctx, (char*)out, &out_msg_len);
  TEST_ASSERT_EQUAL_INT32(SC_OK, rc1);
  ta_cipher_ctx deserialize_ctx = {
      .ciphertext = payload_out,
      .hmac = {0},
      .iv = {0},
  };
  int rc2 = deserialize_msg((char*)out, &deserialize_ctx);
  TEST_ASSERT_EQUAL_INT32(SC_OK, rc2);

  TEST_ASSERT_EQUAL_UINT8_ARRAY(iv, deserialize_ctx.iv, AES_BLOCK_SIZE);
  TEST_ASSERT_EQUAL_UINT64(test_timestamp, deserialize_ctx.timestamp);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(test_hmac, deserialize_ctx.hmac, TA_AES_HMAC_SIZE);
  TEST_ASSERT_EQUAL_UINT32(payload_len, deserialize_ctx.ciphertext_len);
  TEST_ASSERT_EQUAL_UINT8_ARRAY(payload, deserialize_ctx.ciphertext, payload_len);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_serialize_deserialize);

  return UNITY_END();
}
