/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef REQUEST_TA_REGISTER_MAM_CHANNEL_H_
#define REQUEST_TA_REGISTER_MAM_CHANNEL_H_

#include "common/model/transaction.h"
#include "common/ta_errors.h"
#include "utils/containers/hash/hash243_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/request/ta_register_mam_channel.h
 */

/** struct of ta_register_mam_channel_req_t */
typedef struct ta_register_mam_channel_req {
  char seed[NUM_TRYTES_ADDRESS + 1];
} ta_register_mam_channel_req_t;

/**
 * @brief Allocate memory of ta_register_mam_channel_req_t
 *
 * @return
 * - struct of ta_register_mam_channel_req_t on success
 * - NULL on error
 */
ta_register_mam_channel_req_t* ta_register_mam_channel_req_new();

/**
 * @brief Free memory of ta_register_mam_channel_req_t
 *
 * @param[in] req Data type of ta_register_mam_channel_req_t
 */
void ta_register_mam_channel_req_free(ta_register_mam_channel_req_t** req);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_REGISTER_MAM_CHANNEL_H_
