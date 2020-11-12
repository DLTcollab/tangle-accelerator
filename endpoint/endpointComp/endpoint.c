/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdio.h>
#include <stdlib.h>

#include "build/obd_generated.h"

#include "endpoint.h"
#include "hal/device.h"

#include "cipher.h"
#include "common/ta_errors.h"
#include "endpoint_core.h"
#include "le_test.h"
#include "legato.h"

#include "le_log.h"

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
  const char *host = NULL;
  const char *port = NULL;
  const char *ssl_seed = NULL;
  const char *message = NULL;
  const char *mam_seed = NULL;

  char device_id[16] = {0};
  const char *device_id_ptr = device_id;

  uint8_t private_key[AES_CBC_KEY_SIZE] = {0};

  le_arg_SetStringVar(&host, NULL, "host");
  le_arg_SetStringVar(&port, NULL, "port");
  le_arg_SetStringVar(&ssl_seed, NULL, "ssl-seed");
  le_arg_SetStringVar(&message, NULL, "msg");
  le_arg_SetStringVar(&mam_seed, NULL, "mam-seed");
  le_arg_SetFlagCallback(print_help, "h", "help");
  le_arg_Scan();

  srand(time(NULL));

  device_t *device = ta_device(STRINGIZE(EP_TARGET));
  if (device == NULL) {
    LE_ERROR("Can not get specific device");
    exit(EXIT_FAILURE);
  }

  device->op->get_key(private_key);
  device->op->get_device_id(device_id);

  status_t ret = SC_OK;
  LE_INFO("=== ENDPOINT TEST BEGIN ===");

  if (host == NULL || port == NULL || message == NULL || mam_seed == NULL) {
    LE_ERROR("The host, port, mam seed and message should not be NULL");
    exit(EXIT_FAILURE);
  }

  const size_t msg_len = strlen(message);
  ret = send_mam_message(host, port, ssl_seed, mam_seed, (uint8_t *)message, msg_len, private_key, device_id_ptr);
  LE_INFO("Send transaction information return: %d", ret);
  exit(ret == SC_OK ? EXIT_SUCCESS : EXIT_FAILURE);
}
