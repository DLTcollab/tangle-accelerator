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
#include <string.h>
#include <time.h>
#include "endpoint/cipher.h"
#include "tests/test_define.h"

#define MAXLINE 1024

char test_payload1[] = "0123456789abcdef";
const size_t test_paylen1 = 16;

uint8_t test_payload2[] = {
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
const size_t test_paylen2 = 263;

const uint8_t iv_global[AES_IV_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};
const char device_id[IMSI_LEN] = "470010171566423";
const uint8_t key[AES_CBC_KEY_SIZE] = {82,  142, 184, 64,  74, 105, 126, 65,  154, 116, 14,  193, 208, 41,  8,  115,
                                       158, 252, 228, 160, 79, 5,   167, 185, 13,  159, 135, 113, 49,  209, 58, 68};

void setUp(void) {}

void tearDown(void) {}

void test_cipher1(void) {
  uint8_t ciphertext[MAXLINE] = {0};
  uint8_t plaintext[MAXLINE] = {0};
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);

  ta_cipher_ctx encrypt_ctx = {.plaintext = (uint8_t*)test_payload1,
                               .plaintext_len = test_paylen1,
                               .ciphertext = ciphertext,
                               .ciphertext_len = MAXLINE,
                               .device_id = device_id,
                               .iv = {0},
                               .hmac = {0},
                               .key = key,
                               .keybits = TA_AES_KEY_BITS,
                               .timestamp = t.tv_sec};
  // Don't forget to set the initialization vector
  memcpy(encrypt_ctx.iv, iv_global, AES_IV_SIZE);
  TEST_ASSERT_EQUAL_INT32(SC_OK, aes_encrypt(&encrypt_ctx));

  ta_cipher_ctx decrypt_ctx = {.plaintext = plaintext,
                               .plaintext_len = test_paylen1,
                               .ciphertext = encrypt_ctx.ciphertext,
                               .ciphertext_len = encrypt_ctx.ciphertext_len,
                               .device_id = device_id,
                               .iv = {0},
                               .hmac = {0},
                               .key = key,
                               .keybits = TA_AES_KEY_BITS,
                               .timestamp = encrypt_ctx.timestamp};
  // Don't forget to set the initialization vector
  memcpy(decrypt_ctx.iv, encrypt_ctx.iv, AES_IV_SIZE);
  // Don't forget to set the hmac
  memcpy(decrypt_ctx.hmac, encrypt_ctx.hmac, TA_AES_HMAC_SIZE);
  TEST_ASSERT_EQUAL_INT32(SC_OK, aes_decrypt(&decrypt_ctx));

  TEST_ASSERT_EQUAL_INT(test_paylen1, decrypt_ctx.plaintext_len);
  TEST_ASSERT_EQUAL_MEMORY_ARRAY(test_payload1, decrypt_ctx.plaintext, sizeof(uint8_t), test_paylen1);
}

void test_cipher2(void) {
  uint8_t ciphertext[MAXLINE] = {0};
  uint8_t plaintext[MAXLINE] = {0};
  struct timespec t;
  clock_gettime(CLOCK_MONOTONIC, &t);
  ta_cipher_ctx encrypt_ctx = {.plaintext = test_payload2,
                               .plaintext_len = test_paylen2,
                               .ciphertext = ciphertext,
                               .ciphertext_len = MAXLINE,
                               .device_id = device_id,
                               .iv = {0},
                               .hmac = {0},
                               .key = key,
                               .keybits = TA_AES_KEY_BITS,
                               .timestamp = t.tv_sec};
  // Don't forget to set the initialization vector
  memcpy(encrypt_ctx.iv, iv_global, AES_IV_SIZE);
  TEST_ASSERT_EQUAL_INT32(SC_OK, aes_encrypt(&encrypt_ctx));

  ta_cipher_ctx decrypt_ctx = {.plaintext = plaintext,
                               .plaintext_len = test_paylen2,
                               .ciphertext = encrypt_ctx.ciphertext,
                               .ciphertext_len = encrypt_ctx.ciphertext_len,
                               .device_id = device_id,
                               .iv = {0},
                               .hmac = {0},
                               .key = key,
                               .keybits = TA_AES_KEY_BITS,
                               .timestamp = encrypt_ctx.timestamp};
  // Don't forget to set the initialization vector
  memcpy(decrypt_ctx.iv, encrypt_ctx.iv, AES_IV_SIZE);
  // Don't forget to set the hmac
  memcpy(decrypt_ctx.hmac, encrypt_ctx.hmac, TA_AES_HMAC_SIZE);
  TEST_ASSERT_EQUAL_INT32(SC_OK, aes_decrypt(&decrypt_ctx));

  TEST_ASSERT_EQUAL_UINT(test_paylen2, decrypt_ctx.plaintext_len);
  TEST_ASSERT_EQUAL_MEMORY_ARRAY(test_payload2, decrypt_ctx.plaintext, sizeof(uint8_t), test_paylen2);
}

int main(void) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  cipher_logger_init();
  RUN_TEST(test_cipher1);
  RUN_TEST(test_cipher2);
  cipher_logger_release();

  return UNITY_END();
}
