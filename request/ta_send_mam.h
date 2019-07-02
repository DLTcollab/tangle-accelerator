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
#include "accelerator/errors.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file request/ta_send_mam.h
 */

/** struct of ta_send_transfer_req_t */
typedef struct send_mam_req_s {
  tryte_t prng[NUM_TRYTES_ADDRESS + 1];
  char* payload;
  uint8_t channel_ord;
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
