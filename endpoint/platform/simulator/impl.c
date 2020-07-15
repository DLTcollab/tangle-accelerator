/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "endpoint/platform/impl.h"

#include "common/ta_errors.h"
#include "legato.h"

#include "le_hashmap.h"
#include "le_log.h"

#define UART_BUFFER_SIZE 1024

/* Setting data to produce predictable results for simulator */
// device id
static const char *device_id = "470010171566423";

// private key
static const uint8_t private_key[32] = {82,  142, 184, 64,  74, 105, 126, 65,  154, 116, 14,  193, 208, 41,  8,  115,
                                        158, 252, 228, 160, 79, 5,   167, 185, 13,  159, 135, 113, 49,  209, 58, 68};
// Buffer for uart
static char UART_BUFFER[UART_BUFFER_SIZE];

// Hashmap to simulate secure storage
static le_hashmap_Ref_t simulator_sec_table;

void impl_stub_init(void) __attribute__((constructor));

status_t get_device_key(uint8_t *key) {
  memcpy(key, private_key, 16);
  LE_INFO("Get device key success");
  return SC_OK;
}

status_t get_device_id(char *id) {
  memcpy(id, device_id, 16);
  LE_INFO("Get device id success");
  return SC_OK;
}

status_t uart_init(const char *device) {
  LE_INFO("UART init device %s success", device);
  return SC_OK;
}

void uart_write(const int fd, const char *cmd) {
  /* simple output */
  size_t cmd_len = strlen(cmd);
  if (cmd_len > UART_BUFFER_SIZE) {
    LE_ERROR("command too long");
    return;
  }
  snprintf(UART_BUFFER, cmd_len, "%s", cmd);
}

char *uart_read(const int fd) {
  char *response = UART_BUFFER;
  return response;
}

void uart_clean(const int fd) { LE_INFO("UART clean success"); }

status_t sec_init(void) {
  LE_INFO("Initialize secure storage");
  simulator_sec_table =
      le_hashmap_Create("Simulator secure storage", 32, le_hashmap_HashString, le_hashmap_EqualsString);
  return SC_OK;
}

status_t sec_write(const char *name, const uint8_t *buf, size_t buf_size) {
  uint8_t *data = malloc(buf_size);
  if (data == NULL) {
    LE_ERROR("Cannot fetch enough memory");
    return SC_OOM;
  }
  uint8_t *ptr;
  memcpy(data, buf, buf_size);

  LE_INFO("Write %s into secure storage", name);

  // the hashmap will return old value if it is replaced
  ptr = le_hashmap_Put(simulator_sec_table, name, data);
  if (ptr != NULL) {
    free(ptr);
  }
  return SC_OK;
}

status_t sec_read(const char *name, uint8_t *buf, size_t *buf_size) {
  LE_INFO("Read %s from secure storage", name);
  uint8_t *data = le_hashmap_Get(simulator_sec_table, name);

  if (data == NULL) {
    *buf_size = 0;
    return SC_ENDPOINT_SEC_ITEM_NOT_FOUND;
  }

  memcpy(buf, data, *buf_size);
  return SC_OK;
}

status_t sec_delete(const char *name) {
  LE_INFO("Delete %s in secure storage", name);
  uint8_t *data = le_hashmap_Remove(simulator_sec_table, name);

  if (data == NULL) {
    return SC_ENDPOINT_SEC_ITEM_NOT_FOUND;
  }

  free(data);
  return SC_OK;
}

void test_stub_init(void) { status_t sec_init(void); }
