/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef __MAP_MODE_H__
#define __MAP_MODE_H__

#include "mam/api/api.h"

#define MSS_DEPTH 1

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Creates a channel
 *
 * @param api - The API [in,out]
 * @param channel_id - A known channel ID [out]
 * @param depth - depth of merkle tree going to generate [in]
 *
 * @return return code
 */
retcode_t map_channel_create(mam_api_t* const api, tryte_t* const channel_id, const size_t depth);

/**
 * Creates and announce a new channel on header
 *
 * @param api - The API [in,out]
 * @param channel_id - A known channel ID [in]
 * @param bundle - The bundle that the packet will be written into [out]
 * @param msg_id - The msg_id [out]
 * @param new_channel_id - The new channel ID [out]
 *
 * @return return code
 */
retcode_t map_announce_channel(mam_api_t* const api, tryte_t const* const channel_id,
                               bundle_transactions_t* const bundle, trit_t* const msg_id,
                               tryte_t* const new_channel_id);

/**
 * Creates and announce a new endpoint on header
 *
 * @param api - The API [in,out]
 * @param channel_id - A known channel ID [in]
 * @param bundle - The bundle that the packet will be written into [out]
 * @param msg_id - The msg_id [out]
 * @param new_endpoint - The new endpoint [out]
 *
 * @return return code
 */
retcode_t map_announce_endpoint(mam_api_t* const api, tryte_t const* const channel_id,
                                bundle_transactions_t* const bundle, trit_t* const msg_id,
                                tryte_t* const new_endpoint_id);

/**
 * Writes a header only bundle on a channel
 *
 * @param api - The API [in,out]
 * @param channel_id - A known channel ID [in]
 * @param bundle - The bundle that the packet will be written into [out]
 * @param msg_id - The msg_id [out]
 *
 * @return return code
 */
retcode_t map_write_header_on_channel(mam_api_t* const api, tryte_t const* const channel_id,
                                      bundle_transactions_t* const bundle, trit_t* const msg_id);

/**
 * Writes a header only bundle on an endpoint
 *
 * @param api - The API [in,out]
 * @param channel_id - A known channel ID [in]
 * @param endpoint_id - A known endpoint ID [in]
 * @param bundle - The bundle that the packet will be written into [out]
 * @param msg_id - The msg_id [out]
 *
 * @return return code
 */
retcode_t map_write_header_on_endpoint(mam_api_t* const api, tryte_t const* const channel_id,
                                       tryte_t const* const endpoint_id, bundle_transactions_t* const bundle,
                                       trit_t* const msg_id);

/**
 * Writes a packet on a bundle
 *
 * @param api - The API [in,out]
 * @param bundle - The bundle that the packet will be written into [out]
 * @param payload - The payload to write [in]
 * @param msg_id - The msg_id [in]
 * @param is_last_packet - True if this is the last packet to be written [in]
 *
 * @return return code
 */
retcode_t map_write_packet(mam_api_t* const api, bundle_transactions_t* const bundle, char const* const payload,
                           trit_t const* const msg_id, bool is_last_packet);

/**
 * Read the MAM message from a bundle
 *
 * @param api - The API [in,out]
 * @param bundle - The bundle that contains the message [in]
 * @param payload_out - The output playload in ascii [out]
 *
 * @return return code
 */
retcode_t map_api_bundle_read(mam_api_t* const api, bundle_transactions_t* bundle, char** payload_out);
#ifdef __cplusplus
}
#endif

#endif  // __MAP_MODE_H__
