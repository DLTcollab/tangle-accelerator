/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#define READ_BUFFER_SIZE 32

int uart_init();
void uart_write(const int fd, char *cmd);
char *uart_read(const int fd);