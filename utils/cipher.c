/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "cipher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/platform_util.h"

#define MAX_TIMESTAMP_LEN 20

status_t aes_decrypt(ta_cipher_ctx* cipher_ctx) {
  // FIXME: Add logger and some checks here
  mbedtls_aes_context ctx;
  int status;
  char* err;
  uint8_t buf[AES_BLOCK_SIZE];

  /* Create and initialise the context */
  mbedtls_aes_init(&ctx);
  mbedtls_platform_zeroize(cipher_ctx->plaintext, sizeof(cipher_ctx->plaintext));
  mbedtls_platform_zeroize(buf, AES_BLOCK_SIZE);

  /* set decryption key */
  if ((status = mbedtls_aes_setkey_dec(&ctx, cipher_ctx->key, TA_AES_KEY_BITS)) != EXIT_SUCCESS) {
    err = "set aes key failed";
    goto exit;
  }

  // Provide the message to be decrypted, and obtain the plaintext output.
  const size_t ciphertext_len = cipher_ctx->ciphertext_len;
  uint8_t* ciphertext = cipher_ctx->ciphertext;
  uint8_t* plaintext = cipher_ctx->plaintext;
  for (size_t i = 0; i < ciphertext_len; i += AES_BLOCK_SIZE) {
    memset(buf, 0, AES_BLOCK_SIZE);
    int n = (ciphertext_len - i > AES_BLOCK_SIZE) ? AES_BLOCK_SIZE : (int)(ciphertext_len - i);
    memcpy(buf, ciphertext + i, n);
    if ((status = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, AES_BLOCK_SIZE, cipher_ctx->iv, buf, buf)) != 0) {
      err = "aes decrpyt failed";
      goto exit;
    }
    memcpy(plaintext, buf, AES_BLOCK_SIZE);
    plaintext += AES_BLOCK_SIZE;
  }

  /* Clean up */
  mbedtls_aes_free(&ctx);
  return SC_OK;
exit:
  fprintf(stderr, "%s\n", err);
  mbedtls_aes_free(&ctx);
  return SC_UTILS_CIPHER_ERROR;
}

status_t aes_encrypt(ta_cipher_ctx* cipher_ctx) {
  // FIXME: Add logger and some checks here
  char* err = NULL;
  int status = 0;
  uint8_t* plaintext = cipher_ctx->plaintext;
  uint8_t* ciphertext = cipher_ctx->ciphertext;
  size_t plaintext_len = cipher_ctx->plaintext_len;
  size_t ciphertext_len = cipher_ctx->ciphertext_len;

  uint8_t tmp[AES_BLOCK_SIZE];
  uint8_t nonce[IMSI_LEN + MAX_TIMESTAMP_LEN + 1] = {0};
  uint8_t digest[AES_BLOCK_SIZE * 2];
  unsigned char buf[AES_BLOCK_SIZE];
  mbedtls_md_context_t sha_ctx;
  mbedtls_aes_context ctx;

  mbedtls_md_init(&sha_ctx);
  mbedtls_aes_init(&ctx);
  if (mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1) != 0) {
    err = "mbedtls_md_setup error";
    goto exit;
  }

  // Check ciphertext has enough space
  size_t new_len = plaintext_len + (AES_BLOCK_SIZE - plaintext_len % 16);
  if (new_len > ciphertext_len) {
    err = "ciphertext has not enough space";
    goto exit;
  }
  cipher_ctx->ciphertext_len = new_len;
  mbedtls_platform_zeroize(tmp, sizeof(tmp));
  mbedtls_platform_zeroize(digest, sizeof(digest));
  mbedtls_platform_zeroize(ciphertext, sizeof(ciphertext));

  // fetch timestamp
  uint64_t timestamp = time(NULL);
  // concatenate (Device_ID, timestamp)
  snprintf((char*)nonce, IMSI_LEN + MAX_TIMESTAMP_LEN + 1, "%s-%ld", cipher_ctx->device_id, timestamp);
  // hash base data
  mbedtls_md_starts(&sha_ctx);
  mbedtls_md_update(&sha_ctx, digest, AES_BLOCK_SIZE * 2);
  mbedtls_md_update(&sha_ctx, nonce, IMSI_LEN + MAX_TIMESTAMP_LEN);
  mbedtls_md_finish(&sha_ctx, digest);

  for (int i = 0; i < AES_BLOCK_SIZE; ++i) {
    tmp[i] = digest[i] ^ digest[i + AES_BLOCK_SIZE];
  }
  memcpy(cipher_ctx->iv, tmp, AES_BLOCK_SIZE);

  /* set encryption key */
  if ((status = mbedtls_aes_setkey_enc(&ctx, cipher_ctx->key, TA_AES_KEY_BITS)) != 0) {
    err = "set aes key failed";
    goto exit;
  }

  // Encrypt plaintext
  for (size_t i = 0; i < new_len; i += AES_BLOCK_SIZE) {
    memset(buf, 0, AES_BLOCK_SIZE);
    int n = (plaintext_len - i > AES_BLOCK_SIZE) ? AES_BLOCK_SIZE : (int)(plaintext_len - i);
    memcpy(buf, plaintext + i, n);
    if ((status = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, AES_BLOCK_SIZE, tmp, buf, buf)) != 0) {
      err = "aes decrpyt failed";
      goto exit;
    }
    memcpy(ciphertext, buf, AES_BLOCK_SIZE);
    ciphertext += AES_BLOCK_SIZE;
  }

  mbedtls_aes_free(&ctx);
  mbedtls_md_free(&sha_ctx);
  return SC_OK;
exit:
  fprintf(stderr, "%s", err);
  mbedtls_aes_free(&ctx);
  mbedtls_md_free(&sha_ctx);
  return SC_UTILS_CIPHER_ERROR;
}
