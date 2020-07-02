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
#include "endpoint/cipher.h"
#include "endpoint/endpoint_core.h"
#include "le_test.h"
#include "legato.h"

#include "le_log.h"

#define TEST_MESSAGE "THISISMSG9THISISMSG9THISISMSG"
#define TEST_MAM_SEED                                                          \
  "POWEREDBYTANGLEACCELERATOR999999999999999999999999999999999999999999999999" \
  "999999A"

const uint8_t test_iv[AES_IV_SIZE] = {164, 3, 98, 193, 52, 162, 107, 252, 184, 42, 74, 225, 157, 26, 88, 72};

static void print_help(void) {
  puts(
      "NAME\n"
      "endpoint - The endpoint for sending transactions to Tangle-accelerator.\n"
      "\n"
      "SYNOPSIS\n"
      "  endpoint [-h]\n"
      "  endpoint [--help]\n"
      "  endpoint [--mam-seed=\"seed\"]\n"
      "           [--msg=\"message\"]\n"
      "           [--host=\"host\"]\n"
      "           [--port=\"port\"]\n"
      "           [--ssl-seed=\"ssl-seed\"]\n"
      "\n"
      "OPTIONS\n"
      "  -h\n"
      "  --help\n"
      "    Print the information for helping the users. Ignore other arguments.\n"
      "\n"
      "  --mam-seed=\"seed\"\n"
      "    Channel root seed. It is an 81-trytes string.\n"
      "\n"
      "  --msg=\"message\"\n"
      "    Assign the message value with string for send_transaction_information.\n"
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
  const char* host = NULL;
  const char* port = NULL;
  const char* ssl_seed = NULL;
  const char* message = TEST_MESSAGE;
  const char* mam_seed = TEST_MAM_SEED;

  char device_id[16] = {0};
  const char* device_id_ptr = device_id;

  uint8_t private_key[AES_CBC_KEY_SIZE] = {0};
  uint8_t iv[AES_IV_SIZE] = {0};

  le_arg_SetStringVar(&host, NULL, "host");
  le_arg_SetStringVar(&port, NULL, "port");
  le_arg_SetStringVar(&ssl_seed, NULL, "ssl-seed");
  le_arg_SetStringVar(&message, NULL, "msg");
  le_arg_SetStringVar(&mam_seed, NULL, "mam-seed");
  le_arg_SetFlagCallback(print_help, "h", "help");
  le_arg_Scan();

  memcpy(iv, test_iv, AES_IV_SIZE);
  srand(time(NULL));

  device_t* device = ta_device(STRINGIZE(EP_TARGET));
  if (device == NULL) {
    LE_ERROR("Can not get specific device");
    exit(EXIT_FAILURE);
  }

  device->op->get_key(private_key);
  device->op->get_device_id(device_id);

  status_t ret = SC_OK;
  LE_INFO("=== ENDPOINT TEST BEGIN ===");
  ret = send_mam_message(host, port, ssl_seed, mam_seed, message, private_key, device_id_ptr, iv);
  LE_INFO("Send transaction information return: %d", ret);
  exit(ret == SC_OK ? EXIT_SUCCESS : EXIT_FAILURE);
}
