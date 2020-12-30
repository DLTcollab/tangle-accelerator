/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "mbedtls/config.h"
#include "mbedtls/platform.h"

#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdsa.h"
#include "mbedtls/entropy.h"
#include "mbedtls/sha256.h"

#include <string.h>

#include "common/logger.h"
#include "common/ta_errors.h"
#include "randomness.h"

#define SHA256_LEN 32

/**
 * @brief Generate ECDSA key pair
 *
 * @param[in] ctx_sign ECDSA context for signing
 * @param[in] ctr_drbg Counter-mode block-cipher-based Deterministic Random Bit Generator object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ecdsa_gen_key_pair(mbedtls_ecdsa_context *ctx_sign, mbedtls_ctr_drbg_context *ctr_drbg);

/**
 * @brief Compute SHA256
 *
 * @param[in] msg Message is going to be hashed
 * @param[in] msg_len The length of message
 * @param[out] hash The output hash result
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t compute_sha256(unsigned char *msg, const int msg_len, unsigned char *hash);

/**
 * @brief Sign messega with ECDSA
 *
 * @param[in] ctx_sign ECDSA context for signing
 * @param[in] ctr_drbg Counter-mode block-cipher-based Deterministic Random Bit Generator object
 * @param[in] input The input string
 * @param[in] input_len The length of input string
 * @param[out] sig Output signed message
 * @param[out] sig_len The length of signed message
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ecdsa_sign_msg(mbedtls_ecdsa_context *ctx_sign, mbedtls_ctr_drbg_context *ctr_drbg, unsigned char *input,
                        const uint16_t input_len, unsigned char *sig, size_t *sig_len);