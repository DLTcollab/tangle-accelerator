/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "can-utils.h"

// Linux headers
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>

#include <stdio.h>
#include <string.h>

status_t can_open(char* dev, int* fd) {
  int s;
  struct sockaddr_can addr;
  struct ifreq ifr;
  status_t ret = SC_OK;
  if ((s = socket(PF_CAN, SOCK_RAW, CAN_RAW)) < 0) {
    ret = SC_ENDPOINT_CAN_OPEN_ERROR;
    goto exit;
  }

  snprintf(ifr.ifr_name, IFNAMSIZ, "%s", dev);
  ioctl(s, SIOCGIFINDEX, &ifr);

  memset(&addr, 0, sizeof(addr));
  addr.can_family = AF_CAN;
  addr.can_ifindex = ifr.ifr_ifindex;

  if (bind(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
    ret = SC_ENDPOINT_CAN_OPEN_ERROR;
    goto exit;
  }
  *fd = s;
exit:
  if (ret != SC_OK) can_close(s);
  return ret;
}

status_t can_close(int fd) {
  if (close(fd) < 0) {
    return SC_ENDPOINT_CAN_CLOSE_ERROR;
  }
  return SC_OK;
}

status_t can_send(int fd, struct can_frame* frame) {
  if (write(fd, frame, sizeof(struct can_frame)) != sizeof(struct can_frame)) {
    return SC_ENDPOINT_CAN_SEND_ERROR;
  }
  return SC_OK;
}

status_t can_recv(int fd, struct can_frame* frame) {
  if (read(fd, frame, sizeof(struct can_frame)) < 0) {
    return SC_ENDPOINT_CAN_RECV_ERROR;
  }
  return SC_OK;
}
