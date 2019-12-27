/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef REQUEST_TA_SEND_TRANSFER_H_
#define REQUEST_TA_SEND_TRANSFER_H_

#include <stdlib.h>
#include "common/trinary/flex_trit.h"
#include "utils/containers/hash/hash243_queue.h"
#include "utils/containers/hash/hash81_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_send_transfer.h
 */

/** struct of ta_send_transfer_req_t */
typedef struct {
  /** Transfer value */
  int value;
  /** Transfer tag is a 81 long flex trits hash queue. */
  hash81_queue_t tag;
  /** Transfer address is a 243 long flex trits hash queue. */
  hash243_queue_t address;
  /** @name message metadata */
  /* @{ */
  /** Transfer message is a 6561 long flex trits hash array. */
  flex_trit_t message[FLEX_TRIT_SIZE_6561];
  /** message length */
  int msg_len;
  /* @} */
} ta_send_transfer_req_t;

/**
 * Allocate memory of ta_send_transfer_req_t
 *
 * @return
 * - struct of ta_send_transfer_req_t on success
 * - NULL on error
 */
ta_send_transfer_req_t* ta_send_transfer_req_new();

/**
 * Free memory of ta_send_transfer_req_t
 *
 * @param req Data type of ta_send_transfer_req_t
 */
void ta_send_transfer_req_free(ta_send_transfer_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_SEND_TRANSFER_H_
