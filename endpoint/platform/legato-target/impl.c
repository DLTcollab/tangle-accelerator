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
#include "endpoint/cipher.h"

#include "legato.h"

#include "interfaces.h"

static le_sim_Id_t SimId;

status_t set_device_key(const char *key) {
  if (key == NULL) {
    LE_ERROR("Failed to get key");
    return SC_ENDPOINT_SET_KEY_ERROR;
  }

  uint8_t private_key[AES_CBC_KEY_SIZE] = {};
  memcpy(private_key, key, AES_CBC_KEY_SIZE);

  le_result_t result = secStoreGlobal_Write(ENDPOINT_DEVICE_KEY_NAME, private_key, AES_CBC_KEY_SIZE);

  if (result != LE_OK) {
    LE_ERROR("Failed to set private key");
    return SC_ENDPOINT_SET_KEY_ERROR;
  }
  return SC_OK;
}

status_t get_device_key(uint8_t *key) {
  if (key == NULL) {
    LE_ERROR("Failed to get key");
    return SC_ENDPOINT_GET_KEY_ERROR;
  }

  size_t key_size = AES_CBC_KEY_SIZE;
  uint8_t private_key[AES_CBC_KEY_SIZE] = {};

  le_result_t result = secStoreGlobal_Read(ENDPOINT_DEVICE_KEY_NAME, private_key, &key_size);

  switch (result) {
    case LE_OK:
      LE_DEBUG("Success to get device key");
      return SC_OK;
    case LE_OVERFLOW:
    case LE_NOT_FOUND:
    case LE_UNAVAILABLE:
    case LE_FAULT:
    default:
      LE_ERROR("Failed to get device key");
      return SC_ENDPOINT_GET_KEY_ERROR;
  }
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
