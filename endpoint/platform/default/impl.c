/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

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

#include "endpoint/platform/impl.h"

#include "legato.h"

#include "interfaces.h"

#define MAXLINE 1024

#define CBC_IV_SIZE 16
#define IMSI_LEN 15
#define READ_BUFFER_SIZE 32
#define DEFAULT_PORT "/dev/ttyHS0"

static le_sim_Id_t SimId;
/* UART file descriptor */
static int uart_fd;

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

static status_t set_interface_attribs(int fd, int speed) {
  struct termios tty;

  if (tcgetattr(fd, &tty) < 0) {
    LE_ERROR("Error from tcgetattr: %s\n", strerror(errno));
    return SC_ENDPOINT_UART_SET_ATTR;
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
    LE_ERROR("Error from tcsetattr: %s\n", strerror(errno));
    return SC_ENDPOINT_UART_SET_ATTR;
  }
  return SC_OK;
}

status_t uart_init(const char *device) {
  if (device == NULL) device = DEFAULT_PORT;
  int fd;

  fd = open(device, O_RDWR | O_NOCTTY | O_SYNC);
  if (fd < 0) {
    LE_ERROR("Error opening %s: %s\n", device, strerror(errno));
    return SC_ENDPOINT_UART;
  }
  /*baudrate 115200, 8 bits, no parity, 1 stop bit */
  set_interface_attribs(fd, B115200);
  uart_fd = fd;

  return SC_OK;
}

void uart_write(const int fd, const char *cmd) {
  /* simple output */
  ssize_t cmd_len = strlen(cmd);
  ssize_t wlen = write(fd, cmd, cmd_len);
  if (wlen != cmd_len) {
    LE_ERROR("Error from write: %zd, %s\n", wlen, strerror(errno));
  }
  tcdrain(fd); /* delay for output */
}

char *uart_read(const int fd) {
  unsigned char buf[READ_BUFFER_SIZE];
  char *response = NULL;

  ssize_t rdlen = read(fd, buf, sizeof(buf) - 1);
  if (rdlen > 0) {
    LE_DEBUG("uart read: %s", buf);
    response = (char *)malloc(sizeof(char) * rdlen);
    strncpy(response, (char *)buf, READ_BUFFER_SIZE);
  } else if (rdlen < 0) {
    LE_ERROR("Error from read: %zd: %s\n", rdlen, strerror(errno));
  }

  return response;
}

void uart_clean(const int fd) {
  if (tcflush(fd, TCIOFLUSH) != 0) {
    LE_ERROR("tcflush error");
  }
}

status_t sec_init(void) {
  LE_INFO("Initialize secure storage success");
  return SC_OK;
}

status_t sec_write(const char *name, const uint8_t *buf, size_t buf_size) {
  le_result_t result = le_secStore_Write(name, buf, buf_size);

  switch (result) {
    case LE_UNAVAILABLE:
      return SC_ENDPOINT_SEC_UNAVAILABLE;
    case LE_OK:
      return SC_OK;
    default:
      return SC_ENDPOINT_SEC_FAULT;
  }
}

status_t sec_read(const char *name, uint8_t *buf, size_t *buf_size) {
  le_result_t result = le_secStore_Read(name, buf, buf_size);

  switch (result) {
    case LE_NOT_FOUND:
      return SC_ENDPOINT_SEC_ITEM_NOT_FOUND;
    case LE_UNAVAILABLE:
      return SC_ENDPOINT_SEC_UNAVAILABLE;
    case LE_OK:
      return SC_OK;
    default:
      return SC_ENDPOINT_SEC_FAULT;
  }
}

status_t sec_delete(const char *name) {
  le_result_t result = le_secStore_Delete(name);

  switch (result) {
    case LE_NOT_FOUND:
      return SC_ENDPOINT_SEC_ITEM_NOT_FOUND;
    case LE_UNAVAILABLE:
      return SC_ENDPOINT_SEC_UNAVAILABLE;
    case LE_OK:
      return SC_OK;
    default:
      return SC_ENDPOINT_SEC_FAULT;
  }
}
