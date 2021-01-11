/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SHELL_H
#define SHELL_H

/**
 * @file endpoint/shellComp/shell.h
 */

#define SHELL_USING_DEVICE "/dev/ttyHS0"
#define ATCMD_USING_DEVICE "/dev/ttyAT"
#define UART_BAUDRATE B9600
#define AT_BAUDRATE B115200

typedef struct {
  char* name;
  char* desc;
  int (*cmd)(char**);
} command;

#endif  // SHELL_H
