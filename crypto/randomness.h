/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ECDH_COMMON_H
#define ECDH_COMMON_H

#include "common/logger.h"
#include "common/ta_errors.h"
#include "mbedtls/config.h"
#include "mbedtls/ctr_drbg.h"
#include "mbedtls/ecdh.h"
#include "mbedtls/entropy.h"
#include "mbedtls/platform.h"

#ifdef __cplusplus
extern "C" {
#endif

logger_id_t crypto_logger_id;

/**
 * Initialize logger for ECDH
 */
void crypto_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int crypto_logger_release();

typedef struct rand_gen_s {
  mbedtls_entropy_context entropy;
  mbedtls_ctr_drbg_context ctr_drbg;
} rand_gen_t;

/**
 * @brief Initialize mbedtls random number generator
 *
 * @param[in] entropy Entropy contrext for randomess
 * @param[in] ctr_drbg Counter-mode block-cipher-based Deterministic Random Bit Generator object
 * @param[in] rand_seed Random seed for random number generator
 * @param[in] seed_len The length of random seed
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t rand_num_gen_init(mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg, char *rand_seed,
                           uint16_t seed_len);

/**
 * @brief Release random number generator
 *
 * @param[in] entropy Entropy contrext for randomess
 * @param[in] ctr_drbg Counter-mode block-cipher-based Deterministic Random Bit Generator object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
void rand_num_gen_release(mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg);

#ifdef __cplusplus
}
#endif

#endif  // ECDH_COMMON_H
