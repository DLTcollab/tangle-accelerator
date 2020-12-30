/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ECDH_ECDH_H
#define ECDH_ECDH_H

#include "common/logger.h"
#include "common/ta_errors.h"
#include "mbedtls/config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/platform.h"
#include "randomness.h"

#ifdef __cplusplus
extern "C" {
#endif

#define SHARE_DATA_LEN 32

/**
 * @brief Initialize ECDH context and generate ECDH keypair
 *
 * @param[in] ctx ECDH context
 * @param[in] ctr_drbg Counter-mode block-cipher-based Deterministic Random Bit Generator object
 * @param[out] pkey Output public key which would be sent to counterpart
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ecdh_gen_public_key(mbedtls_ecdh_context *ctx, mbedtls_ctr_drbg_context *ctr_drbg, unsigned char *pkey);

/**
 * @brief Compute the shared secret by Diffie–Hellman key exchange protocol
 *
 * @param[in] ctx ECDH context
 * @param[in] ctr_drbg Counter-mode block-cipher-based Deterministic Random Bit Generator object
 * @param[in] input_shared_data The public key sent by counterpart
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ecdh_compute_shared_secret(mbedtls_ecdh_context *ctx, mbedtls_ctr_drbg_context *ctr_drbg,
                                    unsigned char *input_shared_data);

#ifdef __cplusplus
}
#endif

#endif  // ECDH_ECDH_H
