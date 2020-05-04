/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_MAM_CORE_H_
#define CORE_MAM_CORE_H_

#include "accelerator/core/core.h"
#include "common/trinary/flex_trit.h"
#include "common/trinary/tryte_ascii.h"
#include "mam/api/api.h"
#include "mam/mam/mam_channel_t_set.h"
#include "utarray.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/mam_core.h
 */

/**
 * Initialize logger for 'mam_core'
 */
void ta_mam_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int ta_mam_logger_release();

typedef enum mam_mss_key_status_e { ANNOUNCE_CHID, KEY_AVAILABLE } mam_mss_key_status_t;

/**
 * @brief Renew the given bundle
 *
 * @param bundle[in,out] The bundle that will be renewed
 *
 */
void bundle_transactions_renew(bundle_transactions_t** bundle);

/**
 * @brief Initialize a mam_api_t object
 *
 * @param api[in,out] The MAM API object
 * @param iconf[in] IOTA API parameter configurations
 * @param seed[in] Seed of MAM channels. It is an optional choice
 *
 * @return return code
 */
status_t ta_mam_init(mam_api_t* const api, const iota_config_t* const iconf, tryte_t const* const seed);

/**
 * @brief Append Pre-Shared Key or NTRU public key into Pre-Shared Key set or NTRU public key set, respectively.
 *
 * @param psks[in] Pre-Shared Key set
 * @param ntru_pks[in] NTRU public key set
 * @param psk[in] List of Pre-Shared Key
 * @param ntru_pk[in] List of NTRU public key
 *
 * @return status code
 */
status_t ta_set_mam_key(mam_psk_t_set_t* const psks, mam_ntru_pk_t_set_t* const ntru_pks, UT_array const* const psk,
                        UT_array const* const ntru_pk);

/**
 * @brief Write payload to bundle on the smallest secret key.
 *
 * With given channel_depth and endpoint_depth, generate the corresponding Channel ID and Endpoint ID, and write the
 * payload to a bundle. The payload is signed with secret key which has the smallest ordinal number.
 *
 * @param service[in] IRI node end point service
 * @param api[in,out] The MAM API object
 * @param channel_depth[in] Depth of channel merkle tree
 * @param psks[in] Pre-Shared Key set
 * @param ntru_pks[in] NTRU public key set
 * @param payload[in] The message that is going to send with MAM
 * @param chid[out] Channel ID
 * @param bundle[out] The bundle contains the Header and Packets of the current Message
 * @param msg_id[out] Unique Message identifier under each channel
 *
 * @return return code
 */
status_t ta_mam_written_msg_to_bundle(const iota_client_service_t* const service, mam_api_t* const api,
                                      const size_t channel_depth, tryte_t* const chid, mam_psk_t_set_t psks,
                                      mam_ntru_pk_t_set_t ntru_pks, char const* const payload,
                                      bundle_transactions_t** bundle, tryte_t* msg_id,
                                      mam_mss_key_status_t* key_status);

/**
 * @brief Write an announcement to bundle.
 *
 * Write the announcement of the next Channel ID (chid1) or Endpoint ID (epid1). This function would
 * automatically determine which one is going to be generated.
 *
 * @param api[in,out] The MAM API object
 * @param channel_depth[in] Depth of channel merkle tree
 * @param chid[in] Channel ID
 * @param ch_remain_sk[in] The number of MSS channel secret keys that haven't been used
 * @param psks[in] Pre-Shared Key set
 * @param ntru_pks[in] NTRU public key set
 * @param chid1[in] The next Channel ID
 * @param bundle[out] The bundle contains the Header and Packets of the current Message
 *
 * @return return code
 */
status_t ta_mam_write_announce_to_bundle(mam_api_t* const api, const size_t channel_depth,
                                         mam_mss_key_status_t key_status, tryte_t* const chid, mam_psk_t_set_t psks,
                                         mam_ntru_pk_t_set_t ntru_pks, tryte_t* const chid1,
                                         bundle_transactions_t** bundle);

/**
 * @brief Read the MAM message from a bundle
 *
 * @param api[in] The MAM API object
 * @param bundle[in] The bundle that contains the message
 * @param payload_out[out] The output playload in ascii
 *
 * @return status code
 */
status_t ta_mam_api_bundle_read(mam_api_t* const api, bundle_transactions_t* bundle, char** payload_out);

#ifdef __cplusplus
}
#endif

#endif  // CORE_MAM_CORE_H_
