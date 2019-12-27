/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef REQUEST_TA_SEND_MAM_H_
#define REQUEST_TA_SEND_MAM_H_

#include <stdlib.h>
#include <string.h>
#include "common/model/transaction.h"
#include "common/ta_errors.h"
#include "common/trinary/tryte.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_send_mam.h
 */

/** struct of ta_send_mam_req_t */
typedef struct send_mam_req_s {
  /** Optional. MAM channel seed */
  tryte_t* seed;
  /** Optional. The channel ordinal. */
  int32_t channel_ord;
  /** Required. The message will be append to the channel. */
  char* message;
  /** Optional. The depth of channel merkle tree. */
  int32_t ch_mss_depth;
  /** Optional. The depth of endpoint merkle tree. */
  int32_t ep_mss_depth;
  /** Optional. The pre-shared key to encrypt the message. Length: 81 trytes. Default: NULL. */
  tryte_t* psk;
  /** Optional. The NTRU public key to encrypt the message. Length: 1024 trytes. Default: NULL. */
  tryte_t* ntru_pk;
} ta_send_mam_req_t;

/**
 * Allocate memory of ta_send_mam_req_t
 *
 * @return
 * - struct of ta_send_mam_req_t on success
 * - NULL on error
 */
ta_send_mam_req_t* send_mam_req_new();

/**
 * Free memory of ta_send_mam_req_t
 *
 * @param req Data type of ta_send_mam_req_t
 */
void send_mam_req_free(ta_send_mam_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_SEND_MAM_H_
