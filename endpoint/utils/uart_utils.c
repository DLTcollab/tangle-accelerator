/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "uart_utils.h"
#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

static int set_interface_attribs(int fd, int speed) {
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    printf("Error from tcgetattr: %s\n", strerror(errno));
    return -1;
  }

  cfsetospeed(&tty, (speed_t)speed);
  cfsetispeed(&tty, (speed_t)speed);

  tty.c_cflag |= (CLOCAL | CREAD); /* ignore modem controls */
  tty.c_cflag &= ~CSIZE;
  tty.c_cflag |= CS8;      /* 8-bit characters */
  tty.c_cflag &= ~PARENB;  /* no parity bit */
  tty.c_cflag &= ~CSTOPB;  /* only need 1 stop bit */
  tty.c_cflag &= ~CRTSCTS; /* no hardware flowcontrol */

  /* setup for non-canonical mode */
  tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
  tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
  tty.c_oflag &= ~OPOST;

  /* fetch bytes as they become available */
  tty.c_cc[VMIN] = 1;
  tty.c_cc[VTIME] = 1;

  if (tcsetattr(fd, TCSANOW, &tty) != 0) {
    printf("Error from tcsetattr: %s\n", strerror(errno));
    return -1;
  }
  return 0;
}

int uart_init() {
  char *portname = "/dev/ttyHS0";
  int fd;

  fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    printf("Error opening %s: %s\n", portname, strerror(errno));
    return -1;
  }
  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  set_interface_attribs(fd, B115200);

  return fd;
}

void uart_write(const int fd, char *cmd) {
  /* simple output */
  ssize_t cmd_len = strlen(cmd);
  ssize_t wlen = write(fd, cmd, cmd_len);
  if (wlen != cmd_len) {
    printf("Error from write: %ld, %d\n", wlen, errno);
  }
  tcdrain(fd); /* delay for output */
}

char *uart_read(const int fd) {
  unsigned char buf[READ_BUFFER_SIZE];
  char *response = NULL;

  ssize_t rdlen = read(fd, buf, sizeof(buf) - 1);
  if (rdlen > 0) {
    // printf("buf = %s\n", buf);
    response = (char *)malloc(sizeof(char) * rdlen);
    strncpy(response, (char *)buf, READ_BUFFER_SIZE);
  } else if (rdlen < 0) {
    printf("Error from read: %ld: %s\n", rdlen, strerror(errno));
  }

  return response;
}
