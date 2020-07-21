/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_SEND_MAM_H_
#define RESPONSE_TA_SEND_MAM_H_

#include "common/macros.h"
#include "common/model/transaction.h"
#include "common/ta_errors.h"
#include "common/trinary/tryte.h"
#include "mam/mam/message.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/response/ta_send_mam.h
 */

/** struct of ta_send_mam_res_t */
typedef struct send_mam_res_s {
  /** ascii string bundle hash */
  char bundle_hash[NUM_TRYTES_HASH + 1];
  /** ascii string channel id */
  char chid[NUM_TRYTES_HASH + 1];
  /** channel ordinal which is the number of channel we generated */
  char msg_id[NUM_TRYTES_MAM_MSG_ID + 1];
  /** bundle hash of announcement bundle */
  char announcement_bundle_hash[NUM_TRYTES_HASH + 1];
  /** ascii string the next channel id */
  char chid1[NUM_TRYTES_HASH + 1];
} ta_send_mam_res_t;

/**
 * @brief Allocate memory of ta_send_mam_res_t
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
 * @brief Set the msg_id field of send_mam_res object.
 *
 * @param[in] res ta_send_mam_res_t struct object
 * @param[in] msg_id Message id decoded in trytes string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_msg_id(ta_send_mam_res_t* res, const tryte_t* msg_id);

/**
 * @brief Set the announcement_bundle_hash field of send_mam_res object.
 *
 * Receive a announcement bundle hash tryte_t array which is 81 trytes long,
 * and convert (instead of decoding) the received announcement bundle hash to ascii string.
 * After conversion, set the  announcement_bundle_hash field of send_mam_res object.
 *
 * @param[in] res ta_send_mam_res_t struct object
 * @param[in] announcement_bundle_hash announcement bundle hash decoded in trytes string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_announce_bundle_hash(ta_send_mam_res_t* res, const tryte_t* announcement_bundle_hash);

/**
 * @brief Set the next channel_id field of send_mam_res object.
 *
 * Receive a chid1 tryte_t array which is 81 trytes long,
 * and convert (instead of decoding) the received chid1 to ascii string.
 * After conversion, set the  chid1 field of send_mam_res object.
 *
 * @param[in] res ta_send_mam_res_t struct object
 * @param[in] chid1 chid1 decoded in trytes string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_chid1(ta_send_mam_res_t* res, const tryte_t* chid1);

/**
 * @brief Set the content of the response of a MAM message request
 *
 * @param[in,out] res ta_send_mam_res_t struct object
 * @param[in] chid Current Channel ID decoded in trytes string
 * @param[in] msg_id Message ID decoded in trytes string
 * @param[in] bundle Bundle object with MAM message
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_msg_result(ta_send_mam_res_t* res, const tryte_t* chid, const tryte_t* msg_id,
                                     bundle_transactions_t* bundle);
/**
 * @brief Set the content of the response of a MAM announcement
 *
 * @param[in,out] res ta_send_mam_res_t struct object
 * @param[in] chid1 Next Channel ID (chid1) decoded in trytes string
 * @param[in] bundle Bundle object with MAM announcement
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_set_announce(ta_send_mam_res_t* res, const tryte_t* chid1, bundle_transactions_t* bundle);

/**
 * Free memory of ta_send_mam_res_t
 *
 * @param[in] res Pointer of pointer of ta_send_mam_res_t object
 */
void send_mam_res_free(ta_send_mam_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_SEND_MAM_H_
