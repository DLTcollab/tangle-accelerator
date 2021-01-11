/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "cipher.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "common/logger.h"
#include "mbedtls/aes.h"
#include "mbedtls/md.h"
#include "mbedtls/platform_util.h"

#define MAX_TIMESTAMP_LEN 20
#define CIPHER_LOGGER "cipher"

static logger_id_t logger_id;

void cipher_logger_init() { logger_id = logger_helper_enable(CIPHER_LOGGER, LOGGER_DEBUG, true); }

int cipher_logger_release() {
  logger_helper_release(logger_id);
  return SC_OK;
}

static status_t check_cipher_ctx(ta_cipher_ctx* cipher_ctx) {
  char* err = NULL;
  if (cipher_ctx->plaintext == NULL || cipher_ctx->ciphertext == NULL) {
    err = "The plaintext or cipher text inside cipher context cannot be NULL";
    goto exit;
  }

  if (cipher_ctx->device_id == NULL) {
    err = "The device id cannot be NULL";
    goto exit;
  }

  if (cipher_ctx->key == NULL) {
    err = "The device key cannot be NULL";
    goto exit;
  }

  return SC_OK;
exit:
  ta_log_error("%s\n", err);
  return SC_UTILS_CIPHER_ERROR;
}

status_t aes_decrypt(ta_cipher_ctx* cipher_ctx) {
  mbedtls_aes_context ctx;
  mbedtls_md_context_t sha_ctx;
  status_t status;
  char* err = NULL;

  if (check_cipher_ctx(cipher_ctx) != SC_OK) {
    err = "Some attributes inside cipher context cannot be NULL";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }

  uint8_t buf[AES_BLOCK_SIZE];
  uint8_t digest[AES_BLOCK_SIZE * 2];
  uint8_t nonce[IMSI_LEN + MAX_TIMESTAMP_LEN + 1] = {0};
  /* Create and initialise the context */
  mbedtls_aes_init(&ctx);
  mbedtls_md_init(&sha_ctx);
  if (mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1) != 0) {
    err = "Failed to set up message-digest information";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }
  mbedtls_platform_zeroize(cipher_ctx->plaintext, sizeof(cipher_ctx->plaintext));
  mbedtls_platform_zeroize(buf, AES_BLOCK_SIZE);
  mbedtls_platform_zeroize(digest, AES_BLOCK_SIZE * 2);

  /* set decryption key */
  if ((status = mbedtls_aes_setkey_dec(&ctx, cipher_ctx->key, TA_AES_KEY_BITS)) != EXIT_SUCCESS) {
    err = "Failed to set AES key";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }

  // concatenate (Device_ID, timestamp)
  snprintf((char*)nonce, IMSI_LEN + MAX_TIMESTAMP_LEN + 1, "%s-%" PRIu64, cipher_ctx->device_id, cipher_ctx->timestamp);
  // hash base data
  mbedtls_md_starts(&sha_ctx);
  mbedtls_md_update(&sha_ctx, digest, AES_BLOCK_SIZE * 2);
  mbedtls_md_update(&sha_ctx, nonce, IMSI_LEN + MAX_TIMESTAMP_LEN);
  mbedtls_md_update(&sha_ctx, cipher_ctx->key, TA_AES_KEY_BITS / 8);
  mbedtls_md_finish(&sha_ctx, digest);

  mbedtls_md_hmac_starts(&sha_ctx, digest, TA_AES_HMAC_SIZE);

  // Provide the message to be decrypted, and obtain the plaintext output.
  const size_t ciphertext_len = cipher_ctx->ciphertext_len;
  uint8_t* ciphertext = cipher_ctx->ciphertext;
  uint8_t* plaintext = cipher_ctx->plaintext;
  for (size_t i = 0; i < ciphertext_len; i += AES_BLOCK_SIZE) {
    memset(buf, 0, AES_BLOCK_SIZE);
    int n = (ciphertext_len - i > AES_BLOCK_SIZE) ? AES_BLOCK_SIZE : (int)(ciphertext_len - i);
    memcpy(buf, ciphertext + i, n);
    mbedtls_md_hmac_update(&sha_ctx, buf, AES_BLOCK_SIZE);
    if ((status = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_DECRYPT, AES_BLOCK_SIZE, cipher_ctx->iv, buf, buf)) != 0) {
      err = "Failed to decrypt AES message";
      status = SC_UTILS_CIPHER_ERROR;
      goto exit;
    }
    memcpy(plaintext, buf, AES_BLOCK_SIZE);
    plaintext += AES_BLOCK_SIZE;
  }

  // compare hmac
  mbedtls_md_hmac_finish(&sha_ctx, digest);
  if (memcmp(digest, cipher_ctx->hmac, TA_AES_HMAC_SIZE) != 0) {
    err = "Failed to validate HMAC";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }
  status = SC_OK;
exit:
  if (!err) ta_log_error("%s\n", err);
  mbedtls_aes_free(&ctx);
  mbedtls_md_free(&sha_ctx);
  return status;
}

status_t aes_encrypt(ta_cipher_ctx* cipher_ctx) {
  char* err = NULL;
  int status = 0;
  uint8_t* plaintext = cipher_ctx->plaintext;
  uint8_t* ciphertext = cipher_ctx->ciphertext;
  size_t plaintext_len = cipher_ctx->plaintext_len;
  size_t ciphertext_len = cipher_ctx->ciphertext_len;

  if (check_cipher_ctx(cipher_ctx) != SC_OK) {
    err = "Some attributes inside cipher context cannot be NULL";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }

  uint8_t tmp[AES_BLOCK_SIZE];
  uint8_t nonce[IMSI_LEN + MAX_TIMESTAMP_LEN + 1] = {0};
  uint8_t digest[AES_BLOCK_SIZE * 2];
  unsigned char buf[AES_BLOCK_SIZE];
  mbedtls_md_context_t sha_ctx;
  mbedtls_aes_context ctx;

  mbedtls_md_init(&sha_ctx);
  mbedtls_aes_init(&ctx);
  if (mbedtls_md_setup(&sha_ctx, mbedtls_md_info_from_type(MBEDTLS_MD_SHA256), 1) != 0) {
    err = "Failed to set up message-digest information";
    goto exit;
  }

  // Check ciphertext has enough space
  size_t new_len = plaintext_len + (AES_BLOCK_SIZE - plaintext_len % 16);
  if (new_len > ciphertext_len) {
    err = "Failed to get enough space inside ciphertext buffer";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }
  cipher_ctx->ciphertext_len = new_len;
  mbedtls_platform_zeroize(tmp, sizeof(tmp));
  mbedtls_platform_zeroize(digest, sizeof(digest));
  mbedtls_platform_zeroize(ciphertext, sizeof(ciphertext));

  // concatenate (Device_ID, timestamp)
  snprintf((char*)nonce, IMSI_LEN + MAX_TIMESTAMP_LEN + 1, "%s-%" PRIu64, cipher_ctx->device_id, cipher_ctx->timestamp);
  // hash base data
  mbedtls_md_starts(&sha_ctx);
  mbedtls_md_update(&sha_ctx, digest, AES_BLOCK_SIZE * 2);
  mbedtls_md_update(&sha_ctx, nonce, IMSI_LEN + MAX_TIMESTAMP_LEN);
  mbedtls_md_update(&sha_ctx, cipher_ctx->key, TA_AES_KEY_BITS / 8);
  mbedtls_md_finish(&sha_ctx, digest);

  for (int i = 0; i < AES_BLOCK_SIZE; ++i) {
    tmp[i] = digest[i] ^ digest[i + AES_BLOCK_SIZE];
  }
  memcpy(cipher_ctx->iv, tmp, AES_BLOCK_SIZE);

  /* set encryption key */
  if ((status = mbedtls_aes_setkey_enc(&ctx, cipher_ctx->key, TA_AES_KEY_BITS)) != 0) {
    err = "Failed to set AES key";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }

  if ((status = mbedtls_md_hmac_starts(&sha_ctx, digest, TA_AES_HMAC_SIZE)) != 0) {
    err = "Failed to initialize HMAC context";
    status = SC_UTILS_CIPHER_ERROR;
    goto exit;
  }

  // Encrypt plaintext
  for (size_t i = 0; i < new_len; i += AES_BLOCK_SIZE) {
    memset(buf, 0, AES_BLOCK_SIZE);
    int n = (plaintext_len - i > AES_BLOCK_SIZE) ? AES_BLOCK_SIZE : (int)(plaintext_len - i);
    memcpy(buf, plaintext + i, n);
    if ((status = mbedtls_aes_crypt_cbc(&ctx, MBEDTLS_AES_ENCRYPT, AES_BLOCK_SIZE, tmp, buf, buf)) != 0) {
      err = "Failed to encrypt AES message";
      status = SC_UTILS_CIPHER_ERROR;
      goto exit;
    }
    mbedtls_md_hmac_update(&sha_ctx, buf, AES_BLOCK_SIZE);
    memcpy(ciphertext, buf, AES_BLOCK_SIZE);
    ciphertext += AES_BLOCK_SIZE;
  }

  mbedtls_md_hmac_finish(&sha_ctx, digest);
  memcpy(cipher_ctx->hmac, digest, TA_AES_HMAC_SIZE);
  status = SC_OK;
exit:
  if (err != NULL) ta_log_error("%s\n", err);
  mbedtls_aes_free(&ctx);
  mbedtls_md_free(&sha_ctx);
  return status;
}
