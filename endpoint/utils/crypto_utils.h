/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CRYPTO_UTILS_H
#define CRYPTO_UTILS_H

#include <stdint.h>
#include "defined_error.h"

#ifdef __cplusplus
extern "C" {
#endif

#define AES_BLOCK_SIZE 16
#define AES_CBC_KEY_SIZE AES_BLOCK_SIZE * 2
#define MAXLINE 1024
#define IMSI_LEN 15

/**
 * @brief Encrypt plaintext
 *
 * @param[in] plaintext The text to be encrypted
 * @param[in] plaintext_len Plaintext length
 * @param[out] ciphertext Ciphrtext
 * @param[in] ciphertext_len Ciphertext length
 * @param[in, out] iv[AES_BLOCK_SIZE] Initialization vector
 * @param[in] key[AES_CBC_KEY_SIZE] Encryption key
 * @param[in] device_id[IMSI_LEN + 1] Device id
 *
 * @return
 * - Ciphertext text new length on success
 * - 0 on error
 */
int encrypt(const char *plaintext, int plaintext_len, char *ciphertext, int ciphertext_len, uint8_t iv[AES_BLOCK_SIZE],
            uint8_t key[AES_CBC_KEY_SIZE], uint8_t device_id[IMSI_LEN + 1]);

/**
 * @brief Decrypt ciphertext
 *
 * @param[in] ciphertext Ciphertext
 * @param[in] ciphertext_len Ciphertext length
 * @param[out] plaintext Plaintext
 * @param[in] plaintext_len Plaintext length
 * @param[in] iv[AES_BLOCK_SIZE] Initialization vector
 * @param[in] key[AES_CBC_KEY_SIZE] Decryption key
 *
 * @return #retcode_t
 */
retcode_t decrypt(const char *ciphertext, int ciphertext_len, char *plaintext, int plaintext_len,
                  uint8_t iv[AES_BLOCK_SIZE], uint8_t key[AES_CBC_KEY_SIZE]);

/**
 * @brief Implementation of AES encryption algorithm
 *
 * @param[in] plaintext Plaintext
 * @param[in] plaintext_len Plaintext length
 * @param[in] key Encryption key
 * @param[in] keybits Length of key
 * @param[in, out] iv[AES_BLOCK_SIZE] Initialization vector
 * @param[out] ciphertext Ciphertext
 * @param[in] ciphertext_len Ciphertext length
 *
 * @return
 * - Ciphertext new length on success
 * - -1 on error
 */
int aes_encrypt(const char *plaintext, int plaintext_len, const unsigned char *key, unsigned int keybits,
                unsigned char iv[AES_BLOCK_SIZE], char *ciphertext, int ciphertext_len);

/**
 * @brief Implementation of AES decrypt algorithm
 *
 * @param[in] ciphertext Ciphertext
 * @param[in] ciphertext_len Ciphertext length
 * @param[in] key Decryption key
 * @param[in] keybits Length of decryption key
 * @param[in] iv[AES_BLOCK_SIZE] Initialization vector
 * @param[out] plaintext Plaintext
 * @param[in] plaintext_len Plaintext length
 *
 * @return #retcode_t
 */
retcode_t aes_decrypt(const char *ciphertext, int ciphertext_len, const unsigned char *key, unsigned int keybits,
                      unsigned char iv[AES_BLOCK_SIZE], char *plaintext, int plaintext_len);

#ifdef __cplusplus
}
#endif

#endif  // CRYPTO_UTILS_H
