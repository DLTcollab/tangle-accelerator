/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_SEND_MAM_H_
#define RESPONSE_TA_SEND_MAM_H_

#include "accelerator/errors.h"
#include "common/model/transaction.h"
#include "common/trinary/tryte.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_send_mam.h
 */

/** struct of ta_send_mam_res_t */
typedef struct send_mam_res_s {
  /** ascii string bundle hash */
  char bundle_hash[NUM_TRYTES_HASH + 1];
  /** ascii string channel id */
  char channel_id[NUM_TRYTES_HASH + 1];
} ta_send_mam_res_t;

/**
 * Allocate memory of ta_send_mam_res_t
 *
 * @return
 * - struct of ta_send_mam_res_t on success
 * - NULL on error
 */
ta_send_mam_res_t* send_mam_res_new();

/**
 * @brief Set the bundle_hash field of send_mam_res object.
 *
 * Receive a bundle hash tryte_t array which is 81 trytes long,
 * and convert (instead of decoding) the received bundle hash to ascii string.
 * After conversion, set the  bundle_hash field of send_mam_res object.
 *
 * @param[in] res ta_send_mam_res_t struct object
 * @param[in] bundle_hash bundle hash decoded in trytes string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_bundle_hash(ta_send_mam_res_t* res, const tryte_t* bundle_hash);

/**
 * @brief Set the channel_id field of send_mam_res object.
 *
 * Receive a channel id tryte_t array which is 81 trytes long,
 * and convert (instead of decoding) the received channel id to ascii string.
 * After conversion, set the  channel_id field of send_mam_res object.
 *
 * @param[in] res ta_send_mam_res_t struct object
 * @param[in] channel_id channel id decoded in trytes string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_channel_id(ta_send_mam_res_t* res, const tryte_t* channel_id);

/**
 * Free memory of ta_send_mam_res_t
 *
 * @param req Data type of ta_send_mam_res_t
 */
void send_mam_res_free(ta_send_mam_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_SEND_MAM_H_
