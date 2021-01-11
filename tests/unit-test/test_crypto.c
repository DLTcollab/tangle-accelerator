/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "crypto/ecdh.h"
#include "tests/test_define.h"

void test_srv_cli_communication(void) {
  rand_gen_t rand_gen;
  mbedtls_ecdh_context ecdh_srv, ecdh_cli;
  unsigned char cli_to_srv[SHARE_DATA_LEN], srv_to_cli[SHARE_DATA_LEN];

  // initialize ECDH object for server side and client side
  mbedtls_ecdh_init(&ecdh_srv);
  mbedtls_ecdh_init(&ecdh_cli);

  TEST_ASSERT_EQUAL_INT32(SC_OK,
                          rand_gen_init(&rand_gen.entropy, &rand_gen.ctr_drbg, TEST_UUID, strlen(TEST_UUID) + 1));

  // [client] initialize ECDH context and generate public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_gen_public_key(&ecdh_cli, &rand_gen.ctr_drbg, cli_to_srv));

  // [server] initialize ECDH context and generate public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_gen_public_key(&ecdh_srv, &rand_gen.ctr_drbg, srv_to_cli));

  // [server] compute shared secret with peer's public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_compute_shared_secret(&ecdh_srv, &rand_gen.ctr_drbg, cli_to_srv));

  // [client] compute shared secret with peer's public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_compute_shared_secret(&ecdh_cli, &rand_gen.ctr_drbg, srv_to_cli));

  // Check if the two shared secret are the same
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_mpi_cmp_mpi(&ecdh_cli.z, &ecdh_srv.z));

  rand_gen_release(&rand_gen.entropy, &rand_gen.ctr_drbg);
  mbedtls_ecdh_free(&ecdh_srv);
  mbedtls_ecdh_free(&ecdh_cli);
}

int main(void) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  ecdh_logger_init();
  RUN_TEST(test_srv_cli_communication);
  ecdh_logger_release();

  return UNITY_END();
}
