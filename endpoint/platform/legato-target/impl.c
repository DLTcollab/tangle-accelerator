/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "endpoint/platform/impl.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>

#include "legato.h"

#include "interfaces.h"

#define MAXLINE 1024

#define CBC_IV_SIZE 16
#define IMSI_LEN 15
#define READ_BUFFER_SIZE 32
#define DEFAULT_PORT "/dev/ttyHS0"

static le_sim_Id_t SimId;

status_t get_device_key(uint8_t *key) {
  // FIXME:Need to implement the private key generate algorithm
  if (key == NULL) {
    LE_ERROR("Failed to get key");
    return SC_ENDPOINT_GET_KEY_ERROR;
  }

  char *private_key = "AAAAAAAAAAAAAAAA";
  memcpy(key, private_key, CBC_IV_SIZE);

  return SC_OK;
}

static status_t cm_sim_GetSimImsi(char *sim_id) {
  char *imsi = sim_id;
  status_t ret = SC_OK;

  if (le_sim_GetIMSI(SimId, imsi, LE_SIM_IMSI_BYTES) != LE_OK) {
    imsi[0] = '\0';
    ret = SC_ENDPOINT_GET_DEVICE_ID_ERROR;
  }

  return ret;
}

status_t get_device_id(char *device_id) {
  if (cm_sim_GetSimImsi(device_id) != SC_OK) {
    return SC_ENDPOINT_GET_DEVICE_ID_ERROR;
  }
  return SC_OK;
}
