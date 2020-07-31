#include "ecdh.h"

#define logger_id crypto_logger_id

status_t ecdh_gen_public_key(mbedtls_ecdh_context *ctx, mbedtls_ctr_drbg_context *ctr_drbg, unsigned char *pkey) {
  int ret = 1;
  status_t sc = SC_OK;

  ret = mbedtls_ecp_group_load(&ctx->grp, MBEDTLS_ECP_DP_CURVE25519);
  if (ret != 0) {
    ta_log_error("mbedtls_ecp_group_load returned %d\n", ret);
    sc = SC_CRYPTO_GEN_KEY;
    goto exit;
  }

  ret = mbedtls_ecdh_gen_public(&ctx->grp, &ctx->d, &ctx->Q, mbedtls_ctr_drbg_random, ctr_drbg);
  if (ret != 0) {
    ta_log_error("mbedtls_ecdh_gen_public returned %d\n", ret);
    sc = SC_CRYPTO_GEN_KEY;
    goto exit;
  }

  ret = mbedtls_mpi_write_binary(&ctx->Q.X, pkey, SHARE_DATA_LEN);
  if (ret != 0) {
    ta_log_error("mbedtls_mpi_write_binary returned %d\n", ret);
    sc = SC_CRYPTO_GEN_KEY;
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
    sc = SC_CRYPTO_COMPUTE_SECRET;
    goto exit;
  }

  ret = mbedtls_mpi_read_binary(&ctx->Qp.X, input_shared_data, SHARE_DATA_LEN);
  if (ret != 0) {
    ta_log_error("mbedtls_mpi_read_binary returned %d\n", ret);
    sc = SC_CRYPTO_COMPUTE_SECRET;
    goto exit;
  }

  ret = mbedtls_ecdh_compute_shared(&ctx->grp, &ctx->z, &ctx->Qp, &ctx->d, mbedtls_ctr_drbg_random, ctr_drbg);
  if (ret != 0) {
    ta_log_error("mbedtls_ecdh_compute_shared returned %d\n", ret);
    sc = SC_CRYPTO_COMPUTE_SECRET;
  }

exit:
  return sc;
}
