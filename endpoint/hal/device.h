/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef HAL_DEVICE_H
#define HAL_DEVICE_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "defined_error.h"

/*! device initialization entry point */
#define DECLARE_DEVICE(name)                                  \
  void init_##name##_init(void) __attribute__((constructor)); \
  void init_##name##_init(void) { name##_device_type.op->init(); }

typedef struct device_type device_t;

struct device_operations {
  retcode_t (*init)(void);               /**< initialize device */
  void (*fini)(void);                    /**< destructor of device  */
  retcode_t (*get_key)(uint8_t *);       /**< get device private key */
  retcode_t (*get_device_id)(uint8_t *); /**< get device id          */
};

struct uart_operations {
  retcode_t (*init)(const uint8_t *device);     /**< initialize uart */
  void (*write)(const int fd, const char *cmd); /**< write command to uart */
  char *(*read)(const int fd);                  /**< read from uart */
  void (*clean)(const int fd);                  /**< flush uart buffer */
};

struct secure_store_operations {
  retcode_t (*init)(void); /**< initialize secure storage */
  /**
   * @brief Write an item to secure storage
   *
   * @param[in] name Name of item
   * @param[in] buf Pointer to data
   * @param[in] buf_size Length of data
   *
   * @return
   * - #RET_OK on success
   * - #RET_NO_MEMORY on no memory error
   * - #RET_UNAVAILABLE on unavailable secure storage
   * - #RET_FAULT on some other error
   */
  retcode_t (*write)(const char *name, const uint8_t *buf, size_t buf_size);
  /**
   * @brief Read an item from secure storage
   *
   * @param[in] name Name of item
   * @param[out] buf Buffer to store the data in
   * @param[out] buf_size Pointer to length of data
   *
   * @return
   * - #RET_OK on success
   * - #RET_OVERFLOW error on the buffer is too small
   * - #RET_NOT_FOUND error on the item not found inside secure storage
   * - #RET_UNAVAILABLE error if the storage is currently unavailable
   * - #RET_FAULT on some other error
   */
  retcode_t (*read)(const char *name, uint8_t *buf, size_t *buf_size);
  /**
   * @brief Delete item in secure storage
   *
   * @param[in] name Name of item
   *
   * @return
   * - #RET_OK on success
   * - #RET_NOT_FOUND error on the item not found inside secure storage
   * - #RET_UNAVAILABLE error if the storage is currently unavailable
   * - #RET_FAULT if there are some other error
   */
  retcode_t (*delete)(const char *name);
};

struct device_type {
  const char *name;                              /**< device type, a string */
  int uart_fd;                                   /**< uart file descriptor */
  const struct device_operations *op;            /**< device operations handler */
  const struct uart_operations *uart;            /**< uart operations handler */
  const struct secure_store_operations *sec_ops; /**< secure storage operations handler */
  device_t *next;                                /**< pointer to next device type, don't use this directly */
};

/**
 * @brief Obtain specific device handler
 *
 * @param[in] device Device type
 *
 * @return
 * - device handler on success
 * - NULL on failed
 * @see #device_type
 */
device_t *ta_device(const char *device);

/**
 * @brief Register device
 *
 * @param[in] dv Device to register
 *
 * @return
 * - #RET_OK on success
 * - #RET_DEVICE_INIT on failed
 */
retcode_t register_device(struct device_type *dv);

/**
 * @brief Unregister device
 *
 * @param[in] dv Device to unregister
 *
 * @return
 * - #RET_OK on success
 * - #RET_DEVICE_FINI on failed
 */
retcode_t unregister_device(struct device_type *dv);

#ifdef __cplusplus
}
#endif

#endif  // HAL_DEVICE_H
