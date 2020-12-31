/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "endpoint/platform/impl.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common/ta_errors.h"
#include "endpoint/cipher.h"

void simulator_stub_init(void) __attribute__((constructor));

#define ENDPOINT_CONF_NAME "endpoint.conf"

static char device_key[AES_CBC_KEY_SIZE + 1];
static char device_id[IMSI_LEN + 1];

#define ENDPOINT_DEVICE_ID_NAME "Device ID"

#define ENDPOINT_DEVICE_ID_LEN IMSI_LEN
#define ENDPOINT_DEVICE_KEY_LEN AES_CBC_KEY_SIZE

static void gen_random(char* s, const int len) {
  srand(time(NULL));
  static const char alphanum[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
}

static void endpoint_conf_init() {
  memset(device_id, 0, ENDPOINT_DEVICE_ID_LEN + 1);
  memset(device_key, 0, ENDPOINT_DEVICE_KEY_LEN + 1);

  FILE* fp = fopen(ENDPOINT_CONF_NAME, "rb+");
  if (fp) {
    // File exist
    fscanf(fp, ENDPOINT_DEVICE_ID_NAME ":%s\n" ENDPOINT_DEVICE_KEY_NAME ":%s\n", device_id, device_key);

    if (device_id[0] == '\0' || device_key[0] == '\0') {
      LE_ERROR("The device ID or device key is not found. Please remove the " ENDPOINT_CONF_NAME " and try again.");
      fclose(fp);
      exit(EXIT_FAILURE);
    }
  } else {
    // File doesn't exist
    fp = fopen(ENDPOINT_CONF_NAME, "wb+");
    // Use hostname as device ID
    if (gethostname(device_id, ENDPOINT_DEVICE_ID_LEN) < 0) {
      LE_ERROR("Can't get hostname as device ID");
      fclose(fp);
      exit(EXIT_FAILURE);
    }
    gen_random(device_key, ENDPOINT_DEVICE_KEY_LEN);
    fprintf(fp, ENDPOINT_DEVICE_ID_NAME ":%s\n", device_id);
    fprintf(fp, ENDPOINT_DEVICE_KEY_NAME ":%s\n", device_key);
  }

  LE_DEBUG("Device id:%s, Device key:%s", device_id, device_key);
  fclose(fp);
}

status_t set_device_key(const char* key) {
  status_t ret = SC_OK;
  FILE* fp = fopen(ENDPOINT_CONF_NAME, "wb+");

  fscanf(fp, ENDPOINT_DEVICE_ID_NAME ":%s\n" ENDPOINT_DEVICE_KEY_NAME ":%s\n", device_id, device_key);

  memcpy(device_key, key, ENDPOINT_DEVICE_KEY_LEN);
  fprintf(fp, ENDPOINT_DEVICE_ID_NAME ":%s\n", device_id);
  fprintf(fp, ENDPOINT_DEVICE_KEY_NAME ":%s\n", device_key);
  LE_DEBUG("Change key to %s", key);

  fclose(fp);
  return ret;
}

status_t get_device_key(uint8_t* key) {
  memcpy(key, device_key, ENDPOINT_DEVICE_KEY_LEN);
  LE_DEBUG("Get device key: %s", device_key);
  return SC_OK;
}

status_t get_device_id(char* id) {
  memcpy(id, device_id, IMSI_LEN);
  LE_DEBUG("Get device ID: %s", id);
  return SC_OK;
}

void simulator_stub_init(void) { endpoint_conf_init(); }
