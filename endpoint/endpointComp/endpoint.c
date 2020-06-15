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

const uint8_t test_private_key[AES_CBC_KEY_SIZE] = {82,  142, 184, 64,  74,  105, 126, 65,  154, 116, 14,
                                                    193, 208, 41,  8,   115, 158, 252, 228, 160, 79,  5,
                                                    167, 185, 13,  159, 135, 113, 49,  209, 58,  68};
const uint8_t test_iv[AES_IV_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};

static void print_help(void) {
  puts(
      "NAME\n"
      "endpoint - The endpoint for sending transactions to Tangle-accelerator.\n"
      "\n"
      "SYNOPSIS\n"
      "  endpoint [-h]\n"
      "  endpoint [--help]\n"
      "  endpoint [--val=number]\n"
      "           [--msg=\"message\"]\n"
      "           [--msg-fmt=\"message-format\"]\n"
      "           [--tag=\"tag\"]\n"
      "           [--addr=\"address\"]\n"
      "           [--next-addr=\"next-address\"]\n"
      "\n"
      "OPTIONS\n"
      "  -h\n"
      "  --help\n"
      "    Print the information for helping the users. Ignore other arguments.\n"
      "\n"
      "  --val=number\n"
      "    Assign the integer value for send_transaction_information.\n"
      "\n"
      "  --msg=\"message\"\n"
      "    Assign the message value with string for send_transaction_information.\n"
      "\n"
      "  --msg-fmt=\"message-format\"\n"
      "    Assign the message format of --msg with string.\n"
      "\n"
      "  --tag=\"tag\"\n"
      "    Assign the tag value with string for send_transaction_information.\n"
      "    The length should be 27.\n"
      "\n"
      "  --addr=\"address\"\n"
      "    Assign the address with string for send_transaction_information.\n"
      "    The \"address\" should be composed with [9A-Z] and the length should be 81.\n"
      "\n"
      "  --next-addr=\"next-address\"\n"
      "    Assign the next address with string for send_transaction_information.\n"
      "    The \"next-address\" should be composed with [9A-Z] and the length should be 81.\n"
      "\n");

  exit(EXIT_SUCCESS);
}

COMPONENT_INIT {
  // FIXME:
  // The current code is a prototype for passing the CI.
  // The initialization of hardware and the input from hardware are not implemented yet.
  int value = TEST_VALUE;
  const char* message = TEST_MESSAGE;
  const char* message_fmt = TEST_MESSAGE_FMT;
  const char* tag = TEST_TAG;
  const char* address = TEST_ADDRESS;
  const char* next_address = TEST_NEXT_ADDRESS;
  const char* device_id = TEST_DEVICE_ID;
  uint8_t private_key[AES_CBC_KEY_SIZE] = {0};
  uint8_t iv[AES_IV_SIZE] = {0};

  le_arg_SetIntVar(&value, NULL, "val");
  le_arg_SetStringVar(&message, NULL, "msg");
  le_arg_SetStringVar(&message_fmt, NULL, "msg-fmt");
  le_arg_SetStringVar(&tag, NULL, "tag");
  le_arg_SetStringVar(&address, NULL, "addr");
  le_arg_SetStringVar(&next_address, NULL, "next-addr");
  le_arg_SetFlagCallback(print_help, "h", "help");
  le_arg_Scan();

  memcpy(private_key, test_private_key, AES_CBC_KEY_SIZE);
  memcpy(iv, test_iv, AES_IV_SIZE);
  srand(time(NULL));

  while (true) {
    send_transaction_information(value, message, message_fmt, tag, address, next_address, private_key, device_id, iv);
    sleep(10);
  }
}
