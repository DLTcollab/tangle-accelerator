/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_CIPHER_H
#define UTILS_CIPHER_H

#include <stdint.h>
#include <string.h>
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AES_BLOCK_SIZE 16
#define AES_CBC_KEY_SIZE AES_BLOCK_SIZE * 2
#define AES_IV_SIZE AES_BLOCK_SIZE
#define IMSI_LEN 16
#define TA_AES_KEY_BITS 256
#define TA_AES_HMAC_SIZE AES_BLOCK_SIZE * 2

/** context of aes cipher */
typedef struct ta_cipher_ctx {
  uint8_t* plaintext;             /**< Plaintext */
  size_t plaintext_len;           /**< Plaintext length */
  uint8_t* ciphertext;            /**< Ciphertext */
  size_t ciphertext_len;          /**< Ciphertext length */
  uint8_t iv[AES_IV_SIZE];        /**< Initialization vector, mbedtls_aes needs r/w iv[] */
  const uint8_t* key;             /**< Encryption key */
  const size_t keybits;           /**< Bits of key, valid options are 128,192,256 bits */
  const char* device_id;          /**< Device id */
  uint8_t hmac[TA_AES_HMAC_SIZE]; /**< r/w buffer for hash-based message authentication code */
  uint64_t timestamp;             /**< Timestamp for generate hmac */
} ta_cipher_ctx;

/**
 * @brief Initialize logger of cipher
 */
void cipher_logger_init();

/**
 * @brief Release logger of cipher
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int cipher_logger_release();

/**
 * @brief Encrypt plaintext
 *
 * @param[in] ctx The ta_cipher_ctx to be encrypted
 *
 * @return #status_t
 */
status_t aes_encrypt(ta_cipher_ctx* cipher_ctx);

/**
 * @brief Decrypt ciphertext
 *
 * @param[in] ctx The ta_cipher_ctx to be decrypted
 *
 * @return #status_t
 */
status_t aes_decrypt(ta_cipher_ctx* cipher_ctx);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_CIPHER_H
