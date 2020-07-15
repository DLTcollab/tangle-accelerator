/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <stddef.h>
#include "common/ta_errors.h"

status_t get_device_key(uint8_t *key);

status_t get_device_id(char *id);

status_t uart_init(const char *device);

void uart_write(const int fd, const char *cmd);

char *uart_read(const int fd);

void uart_clean(const int fd);

status_t sec_init(void);

status_t sec_write(const char *name, const uint8_t *buf, size_t buf_size);

status_t sec_read(const char *name, uint8_t *buf, size_t *buf_size);

status_t sec_delete(const char *name);
