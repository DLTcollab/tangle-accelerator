#include "randomness.h"

#define CRYPTO_LOGGER "crypto"
#define logger_id crypto_logger_id

void crypto_logger_init() { logger_id = logger_helper_enable(CRYPTO_LOGGER, LOGGER_DEBUG, true); }

int crypto_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

status_t rand_num_gen_init(mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg, char *rand_seed,
                           uint16_t seed_len) {
  int ret = 1;
  status_t sc = SC_OK;

  mbedtls_ctr_drbg_init(ctr_drbg);
  mbedtls_entropy_init(entropy);

  if ((ret = mbedtls_ctr_drbg_seed(ctr_drbg, mbedtls_entropy_func, entropy, (const unsigned char *)rand_seed,
                                   seed_len)) != 0) {
    ta_log_error("mbedtls_ctr_drbg_seed returned %d\n", ret);
    sc = SC_CRYPTO_RAND_INIT;
  }

  return sc;
}

void rand_num_gen_release(mbedtls_entropy_context *entropy, mbedtls_ctr_drbg_context *ctr_drbg) {
  mbedtls_ctr_drbg_free(ctr_drbg);
  mbedtls_entropy_free(entropy);
}
