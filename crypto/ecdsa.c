#include "ecdsa.h"

#define logger_id crypto_logger_id

static void dump_buf(char *title, unsigned char *buf, const int len) {
  char dump[len + 1];
  for (int i = 0; i < len; i++) {
    dump[i * 2] = "0123456789ABCDEF"[buf[i] / 16];
    dump[i * 2 + 1] = "0123456789ABCDEF"[buf[i] % 16];
  }
  ta_log_debug("%s%s\n", title, dump);
}

status_t ecdsa_gen_key_pair(mbedtls_ecdsa_context *ctx_sign, mbedtls_ctr_drbg_context *ctr_drbg) {
  int ret = 1;
  status_t sc = SC_OK;

  if ((ret = mbedtls_ecdsa_genkey(ctx_sign, MBEDTLS_ECP_DP_SECP192R1, mbedtls_ctr_drbg_random, ctr_drbg)) != 0) {
    ta_log_error("mbedtls_ecdsa_genkey returned %d\n", ret);
    sc = SC_CRYPTO_GEN_KEY;
  }

  return sc;
}

status_t compute_sha256(unsigned char *msg, const int msg_len, unsigned char *hash) {
  int ret = 1;
  status_t sc = SC_OK;
  if ((ret = mbedtls_sha256_ret(msg, msg_len, hash, 0)) != 0) {
    ta_log_error("mbedtls_sha256_ret returned %d\n", ret);
    sc = SC_CRYPTO_HASH;
  }

  return sc;
}

status_t ecdsa_sign_msg(mbedtls_ecdsa_context *ctx_sign, mbedtls_ctr_drbg_context *ctr_drbg, unsigned char *hash,
                        const uint16_t hash_len, unsigned char *sig, size_t *sig_len) {
  int ret = 1;
  status_t sc = SC_OK;
  if ((ret = mbedtls_ecdsa_write_signature(ctx_sign, MBEDTLS_MD_SHA256, hash, hash_len, sig, sig_len,
                                           mbedtls_ctr_drbg_random, ctr_drbg)) != 0) {
    ta_log_error("mbedtls_ecdsa_write_signature returned %d\n", ret);
    sc = SC_CRYPTO_ECDSA_SIGN;
  }

  return sc;
}
