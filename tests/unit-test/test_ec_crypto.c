/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "crypto/ecdh.h"
#include "crypto/ecdsa.h"
#include "tests/test_define.h"

#define MSG_LEN 100

void test_srv_cli_communication(void) {
  rand_gen_t rand_gen;
  mbedtls_ecdh_context ecdh_srv, ecdh_cli;
  unsigned char cli_to_srv[SHARE_DATA_LEN], srv_to_cli[SHARE_DATA_LEN];

  // initialize ECDH object for server side and client side
  mbedtls_ecdh_init(&ecdh_srv);
  mbedtls_ecdh_init(&ecdh_cli);

  TEST_ASSERT_EQUAL_INT32(SC_OK,
                          rand_num_gen_init(&rand_gen.entropy, &rand_gen.ctr_drbg, TEST_UUID, strlen(TEST_UUID) + 1));

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

  rand_num_gen_release(&rand_gen.entropy, &rand_gen.ctr_drbg);
  mbedtls_ecdh_free(&ecdh_srv);
  mbedtls_ecdh_free(&ecdh_cli);
}

void test_ecdsa(void) {
  rand_gen_t rand_gen;
  mbedtls_ecdsa_context ctx_sign, ctx_verify;
  unsigned char message[MSG_LEN];
  unsigned char hash[SHA256_LEN] = {};
  unsigned char signature[MBEDTLS_ECDSA_MAX_LEN] = {};
  size_t signature_len = 0;
  memset(message, 0x25, sizeof(message));
  mbedtls_ecdsa_init(&ctx_sign);
  mbedtls_ecdsa_init(&ctx_verify);

  TEST_ASSERT_EQUAL_INT32(SC_OK,
                          rand_num_gen_init(&rand_gen.entropy, &rand_gen.ctr_drbg, TEST_UUID, strlen(TEST_UUID) + 1));

  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdsa_gen_key_pair(&ctx_sign, &rand_gen.ctr_drbg));
  TEST_ASSERT_EQUAL_INT32(SC_OK, compute_sha256(message, MSG_LEN, hash));
  TEST_ASSERT_EQUAL_INT32(SC_OK,
                          ecdsa_sign_msg(&ctx_sign, &rand_gen.ctr_drbg, hash, SHA256_LEN, signature, &signature_len));
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecp_group_copy(&ctx_verify.grp, &ctx_sign.grp));
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecp_copy(&ctx_verify.Q, &ctx_sign.Q));
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecdsa_read_signature(&ctx_verify, hash, sizeof(hash), signature, signature_len));

  rand_num_gen_release(&rand_gen.entropy, &rand_gen.ctr_drbg);
  mbedtls_ecdsa_free(&ctx_verify);
  mbedtls_ecdsa_free(&ctx_sign);
}

void test_sign_ecdh_pkey(void) {
  rand_gen_t rand_gen;
  mbedtls_ecdh_context ecdh_srv, ecdh_cli;
  mbedtls_ecdsa_context cli_ecdsa_sign, cli_ecdsa_verify;
  mbedtls_ecdsa_context srv_ecdsa_sign, srv_ecdsa_verify;
  unsigned char cli_to_srv[SHARE_DATA_LEN], srv_to_cli[SHARE_DATA_LEN];
  unsigned char cli_signature[MBEDTLS_ECDSA_MAX_LEN] = {}, srv_signature[MBEDTLS_ECDSA_MAX_LEN] = {};
  size_t cli_signature_len = 0, srv_signature_len = 0;

  // initialize ECDH object for server side and client side
  mbedtls_ecdh_init(&ecdh_srv);
  mbedtls_ecdh_init(&ecdh_cli);

  // Initialize ECDSA
  mbedtls_ecdsa_init(&cli_ecdsa_sign);
  mbedtls_ecdsa_init(&cli_ecdsa_verify);
  mbedtls_ecdsa_init(&srv_ecdsa_sign);
  mbedtls_ecdsa_init(&srv_ecdsa_verify);

  TEST_ASSERT_EQUAL_INT32(SC_OK,
                          rand_num_gen_init(&rand_gen.entropy, &rand_gen.ctr_drbg, TEST_UUID, strlen(TEST_UUID) + 1));
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdsa_gen_key_pair(&cli_ecdsa_sign, &rand_gen.ctr_drbg));
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdsa_gen_key_pair(&srv_ecdsa_sign, &rand_gen.ctr_drbg));

  // [client] initialize ECDH context and generate public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_gen_public_key(&ecdh_cli, &rand_gen.ctr_drbg, cli_to_srv));

  // Sign client's pkey
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdsa_sign_msg(&cli_ecdsa_sign, &rand_gen.ctr_drbg, cli_to_srv, SHARE_DATA_LEN,
                                                cli_signature, &cli_signature_len));

  // [server] initialize ECDH context and generate public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_gen_public_key(&ecdh_srv, &rand_gen.ctr_drbg, srv_to_cli));

  // Sign service's pkey
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdsa_sign_msg(&srv_ecdsa_sign, &rand_gen.ctr_drbg, srv_to_cli, SHARE_DATA_LEN,
                                                srv_signature, &srv_signature_len));

  // [server] examine if the signature is correct
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecp_group_copy(&cli_ecdsa_verify.grp, &cli_ecdsa_sign.grp));
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecp_copy(&cli_ecdsa_verify.Q, &cli_ecdsa_sign.Q));
  TEST_ASSERT_EQUAL_INT32(
      0, mbedtls_ecdsa_read_signature(&cli_ecdsa_verify, cli_to_srv, SHARE_DATA_LEN, cli_signature, cli_signature_len));

  // [server] compute shared secret with peer's public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_compute_shared_secret(&ecdh_srv, &rand_gen.ctr_drbg, cli_to_srv));

  // [client] examine if the signature is correct
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecp_group_copy(&cli_ecdsa_verify.grp, &cli_ecdsa_sign.grp));
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_ecp_copy(&cli_ecdsa_verify.Q, &cli_ecdsa_sign.Q));
  TEST_ASSERT_EQUAL_INT32(
      0, mbedtls_ecdsa_read_signature(&cli_ecdsa_verify, cli_to_srv, SHARE_DATA_LEN, cli_signature, cli_signature_len));

  // [client] compute shared secret with peer's public key
  TEST_ASSERT_EQUAL_INT32(SC_OK, ecdh_compute_shared_secret(&ecdh_cli, &rand_gen.ctr_drbg, srv_to_cli));

  // Check if the two shared secret are the same
  TEST_ASSERT_EQUAL_INT32(0, mbedtls_mpi_cmp_mpi(&ecdh_cli.z, &ecdh_srv.z));

  rand_num_gen_release(&rand_gen.entropy, &rand_gen.ctr_drbg);
  mbedtls_ecdh_free(&ecdh_srv);
  mbedtls_ecdh_free(&ecdh_cli);
}

int main(void) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  crypto_logger_init();
  RUN_TEST(test_srv_cli_communication);
  RUN_TEST(test_ecdsa);
  RUN_TEST(test_sign_ecdh_pkey);
  crypto_logger_release();

  return UNITY_END();
}
