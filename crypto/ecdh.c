/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "crypto/ecdh.h"

#define ECDH_LOGGER "ecdh"
static logger_id_t logger_id;

void ecdh_logger_init() { logger_id = logger_helper_enable(ECDH_LOGGER, LOGGER_DEBUG, true); }

int ecdh_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

status_t rand_gen_init(mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg, char *rand_seed,
                       uint16_t seed_len) {
  int ret = 1;
  status_t sc = SC_OK;

  mbedtls_ctr_drbg_init(ctr_drbg);
  mbedtls_entropy_init(entropy);

  if ((ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy, (const unsigned char *)rand_seed,
                                   seed_len)) != 0) {
    ta_log_error("mbedtls_ctr_drbg_seed returned %d\n", ret);
    sc = SC_CRYPTO_RAND_ERR;
  }

  return sc;
}

status_t ecdh_gen_public_key(mbedtls_ecdh_context *ctx, mbedtls_ctr_drbg_context *ctr_drbg, unsigned char *pkey) {
  int ret = 1;
  status_t sc = SC_OK;

  ret = mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_CURVE25519);
  if (ret != 0) {
    ta_log_error("mbedtls_ecp_group_load returned %d\n", ret);
    sc = SC_CRYPTO_GENKEY_ERR;
    goto exit;
  }

  ret = mbedtls_ecdh_gen_public(&ctx->grp, &ctx->d, &ctx->Q, mbedtls_ctr_drbg_random, ctr_drbg);
  if (ret != 0) {
    ta_log_error("mbedtls_ecdh_gen_public returned %d\n", ret);
    sc = SC_CRYPTO_GENKEY_ERR;
    goto exit;
  }

  ret = mbedtls_mpi_write_binary(&ctx->Q.X, pkey, SHARE_DATA_LEN);
  if (ret != 0) {
    ta_log_error("mbedtls_mpi_write_binary returned %d\n", ret);
    sc = SC_CRYPTO_GENKEY_ERR;
  }

exit:
  return sc;
}

status_t ecdh_compute_shared_secret(mbedtls_ecdh_context *ctx, mbedtls_ctr_drbg_context *ctr_drbg,
                                    unsigned char *input_shared_data) {
  int ret = 1;
  status_t sc = SC_OK;

  ret = mbedtls_mpi_lset(&ctx->Qp.Z, 1);
  if (ret != 0) {
    ta_log_error("mbedtls_mpi_lset returned %d\n", ret);
    sc = SC_CRYPTO_SECRET_ERR;
    goto exit;
  }

  ret = mbedtls_mpi_read_binary(&ctx->Qp.X, input_shared_data, SHARE_DATA_LEN);
  if (ret != 0) {
    ta_log_error("mbedtls_mpi_read_binary returned %d\n", ret);
    sc = SC_CRYPTO_SECRET_ERR;
    goto exit;
  }

  ret = mbedtls_ecdh_compute_shared(&ctx->grp, &ctx->z, &ctx->Qp, &ctx->d, mbedtls_ctr_drbg_random, ctr_drbg);
  if (ret != 0) {
    ta_log_error("mbedtls_ecdh_compute_shared returned %d\n", ret);
    sc = SC_CRYPTO_SECRET_ERR;
  }

exit:
  return sc;
}

void rand_gen_release(mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg) {
  mbedtls_ctr_drbg_free(ctr_drbg);
  mbedtls_entropy_free(entropy);
}
