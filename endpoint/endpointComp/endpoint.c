/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "endpoint.h"
#include "hal/device.h"

#include "common/ta_errors.h"
#include "endpoint/endpoint_core.h"
#include "le_test.h"
#include "legato.h"
#include "utils/cipher.h"

#include "le_log.h"

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
      "           [--host=\"host\"]\n"
      "           [--port=\"port\"]\n"
      "           [--ssl-seed=\"ssl-seed\"]\n"
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
      "\n"
      "  --host=\"host\"\n"
      "    Assign the host of the tangle-accelerator for send_transaction_information.\n"
      "    The default value is NULL.\n"
      "\n"
      "  --port=\"port\"\n"
      "    Assign the port of the tangle-accelerator for send_transaction_information.\n"
      "    The default value is NULL.\n"
      "\n"
      "  --ssl-seed=\"ssl-seed\"\n"
      "    Assign the random seed of the SSL configuration for send_transaction_information.\n"
      "    The default value is NULL.\n"
      "\n");

  exit(EXIT_SUCCESS);
}

COMPONENT_INIT {
  // FIXME:
  // The current code is a prototype for passing the CI.
  // The initialization of hardware and the input from hardware are not implemented yet.
  int value = TEST_VALUE;
  const char* host = NULL;
  const char* port = NULL;
  const char* ssl_seed = NULL;
  const char* message = TEST_MESSAGE;
  const char* message_fmt = TEST_MESSAGE_FMT;
  const char* tag = TEST_TAG;
  const char* address = TEST_ADDRESS;
  const char* next_address = TEST_NEXT_ADDRESS;

  char device_id[16] = {0};
  const char* device_id_ptr = device_id;

  uint8_t private_key[AES_CBC_KEY_SIZE] = {0};
  uint8_t iv[AES_IV_SIZE] = {0};

  le_arg_SetIntVar(&value, NULL, "val");
  le_arg_SetStringVar(&host, NULL, "host");
  le_arg_SetStringVar(&port, NULL, "port");
  le_arg_SetStringVar(&ssl_seed, NULL, "ssl-seed");
  le_arg_SetStringVar(&message, NULL, "msg");
  le_arg_SetStringVar(&message_fmt, NULL, "msg-fmt");
  le_arg_SetStringVar(&tag, NULL, "tag");
  le_arg_SetStringVar(&address, NULL, "addr");
  le_arg_SetStringVar(&next_address, NULL, "next-addr");
  le_arg_SetFlagCallback(print_help, "h", "help");
  le_arg_Scan();

  memcpy(iv, test_iv, AES_IV_SIZE);
  srand(time(NULL));

  device_t* device = ta_device(STRINGIZE(TARGET));
  device->op->get_key(private_key);
  device->op->get_device_id(device_id);

#ifdef ENABLE_ENDPOINT_TEST
  LE_TEST_INIT;
  LE_TEST_INFO("=== ENDPOINT TEST BEGIN ===");
  LE_TEST(SC_OK == send_transaction_information(host, port, ssl_seed, value, message, message_fmt, tag, address,
                                                next_address, private_key, device_id_ptr, iv));
  LE_TEST_EXIT;
#else
  while (true) {
    status_t ret = send_transaction_information(host, port, ssl_seed, value, message, message_fmt, tag, address,
                                                next_address, private_key, device_id_ptr, iv);
    LE_INFO("Send transaction information return: %d", ret);
    sleep(10);
  }
#endif
}
