/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "endpoint.h"

#include "legato.h"

#include "endpoint/endpoint_core.h"
#include "utils/cipher.h"

#define TEST_VALUE 0
#define TEST_MESSAGE "THISISMSG9THISISMSG9THISISMSG"
#define TEST_MESSAGE_FMT "ascii"
#define TEST_TAG "POWEREDBYTANGLEACCELERATOR9"
#define TEST_ADDRESS                                                           \
  "POWEREDBYTANGLEACCELERATOR999999999999999999999999999999999999999999999999" \
  "999999A"
#define TEST_NEXT_ADDRESS                                                      \
  "POWEREDBYTANGLEACCELERATOR999999999999999999999999999999999999999999999999" \
  "999999B"
#define TEST_DEVICE_ID "470010171566423"

const uint8_t test_key[32] = {82,  142, 184, 64,  74, 105, 126, 65,  154, 116, 14,  193, 208, 41,  8,  115,
                              158, 252, 228, 160, 79, 5,   167, 185, 13,  159, 135, 113, 49,  209, 58, 68};
const uint8_t test_iv[AES_IV_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};

static void print_help(void) {
  puts(
      "NAME\n"
      "endpoint - The endpoint for sending transactions to Tangle-accelerator.\n"
      "\n"
      "SYNOPSIS\n"
      "  endpoint [-h]\n"
      "  endpoint [--help]\n"
      "\n"
      "OPTIONS\n"
      "  -h\n"
      "  --help\n"
      "    Print the information for helping the users. Ignore other arguments.\n"
      "\n");

  exit(EXIT_SUCCESS);
}

COMPONENT_INIT {
  // FIXME:
  // The current code is a prototype for passing the CI.
  // The initialization of hardware and the input from hardware are not implemented yet.
  le_arg_SetFlagCallback(print_help, "h", "help");
  le_arg_Scan();

  uint8_t iv[AES_IV_SIZE] = {0};

  memcpy(iv, test_iv, AES_IV_SIZE);
  srand(time(NULL));

  while (true) {
    send_transaction_information(TEST_VALUE, TEST_MESSAGE, TEST_MESSAGE_FMT, TEST_TAG, TEST_ADDRESS, TEST_NEXT_ADDRESS,
                                 test_key, TEST_DEVICE_ID, iv);
    sleep(10);
  }
}
