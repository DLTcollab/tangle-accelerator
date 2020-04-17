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
 * Initialize a mam_api_t object
 *
 * @param api - The API [in,out]
 * @param[in] iconf IOTA API parameter configurations[in]
 * @param seed - MAM channels seed. It is an optional choice[in]
 *
 * @return return code
 */
status_t ta_mam_init(mam_api_t* const api, const iota_config_t* const iconf, tryte_t const* const seed,
                     mam_psk_t_set_t* const psks, mam_ntru_pk_t_set_t* const ntru_pks, tryte_t const* const psk,
                     tryte_t const* const ntru_pk);

/**
 * @brief Write payload to bundle on the smallest secret key.
 *
 * With given channel_depth and endpoint_depth, generate the corresponding Channel ID and Endpoint ID, and write the
 * payload to a bundle. The payload is signed with secret key which has the smallest ordinal number.
 *
 * @param service - IRI node end point service[in]
 * @param api - The API [in,out]
 * @param channel_depth - Depth of channel merkle tree[in]
 * @param endpoint_depth - Depth of endpoint merkle tree[in]
 * @param psks - Pre-Shared Key set[in]
 * @param ntru_pks - NTRU public key set[in]
 * @param payload - The message that is going to send with MAM[in]
 * @param chid - Channel ID[out]
 * @param epid - Endpoint ID[out]
 * @param bundle - The bundle contains the Header and Packets of the current Message[out]
 * @param msg_id - Unique Message identifier under each channel[out]
 * @param ch_remain_sk - The number of MSS channel secret keys that haven't been used[out]
 * @param ep_remain_sk - The number of MSS endpoint secret keys of that haven't been used[out]
 *
 * @return return code
 */
status_t ta_mam_written_msg_to_bundle(const iota_client_service_t* const service, mam_api_t* const api,
                                      const size_t channel_depth, const size_t endpoint_depth, tryte_t* const chid,
                                      tryte_t* const epid, mam_psk_t_set_t psks, mam_ntru_pk_t_set_t ntru_pks,
                                      char const* const payload, bundle_transactions_t** bundle, tryte_t* msg_id,
                                      uint32_t* ch_remain_sk, uint32_t* ep_remain_sk);

/**
 * @brief Write an announcement to bundle.
 *
 * Write the announcement of the next Channel ID (chid1) or Endpoint ID (epid1). This function would
 * automatically determine which one is going to be generated.
 *
 * @param api - The API [in,out]
 * @param channel_depth - Depth of channel merkle tree[in]
 * @param endpoint_depth - Depth of endpoint merkle tree[in]
 * @param chid - Channel ID[in]
 * @param ch_remain_sk - The number of MSS channel secret keys that haven't been used[in]
 * @param ep_remain_sk - The number of MSS endpoint secret keys of that haven't been used[in]
 * @param psks - Pre-Shared Key set[in]
 * @param ntru_pks - NTRU public key set[in]
 * @param chid1 - The next Channel ID[in]
 * @param bundle - The bundle contains the Header and Packets of the current Message[out]
 *
 * @return return code
 */
status_t ta_mam_announce_next_mss_private_key_to_bundle(mam_api_t* const api, const size_t channel_depth,
                                                        const size_t endpoint_depth, tryte_t* const chid,
                                                        uint32_t* ch_remain_sk, uint32_t* ep_remain_sk,
                                                        mam_psk_t_set_t psks, mam_ntru_pk_t_set_t ntru_pks,
                                                        tryte_t* const chid1, bundle_transactions_t** bundle);

/**
 * Read the MAM message from a bundle
 *
 * @param api - The API [in,out]
 * @param bundle - The bundle that contains the message [in]
 * @param payload_out - The output playload in ascii [out]
 *
 * @return return code
 */
retcode_t ta_mam_api_bundle_read(mam_api_t* const api, bundle_transactions_t* bundle, char** payload_out);

#ifdef __cplusplus
}
#endif

#endif  // CORE_MAM_CORE_H_
