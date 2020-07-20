/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CAN_UTILS_H
#define CAN_UTILS_H

/**
 * @file endpoint/OBDComp/can-bus/can-utils.h
 */

#include "common/ta_errors.h"

#include <linux/can.h>
#include <linux/can/raw.h>

/**
 * @brief Open CAN BUS socket
 *
 * @param[in] dev CAN BUS device
 * @param[out] fd File descriptor
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_CAN_OPEN_ERROR on failed
 */
status_t can_open(char* dev, int* fd);

/**
 * @brief Receive CAN BUS packet
 *
 * @param[in] fd File descriptor
 * @param[in] frame The buffer to store CAN BUS packet
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_CAN_RECV_ERROR on failed
 */
status_t can_recv(int fd, struct can_frame* frame);

/**
 * @brief Send CAN BUS packet
 *
 * @param[in] fd File descriptor
 * @param[in] frame The CAN BUS packet to be sent
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_CAN_SEND_ERROR on failed
 */
status_t can_send(int fd, struct can_frame* frame);

/**
 * @brief Close CAN BUS file descriptor
 *
 * @param[in] fd File descriptor
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_CAN_CLOSE_ERROR on failed
 */
status_t can_close(int fd);

#endif  // CAN_UTILS_H
