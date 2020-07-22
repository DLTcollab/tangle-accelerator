/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_CORE_H_
#define CORE_CORE_H_

#include "accelerator/config.h"
#include "accelerator/core/request/request.h"
#include "accelerator/core/response/response.h"
#include "accelerator/core/serializer/serializer.h"
#include "common/debug.h"
#include "common/model/transfer.h"
#include "utils/bundle_array.h"
#include "utils/char_buffer_str.h"
#include "utils/containers/hash/hash243_set.h"
#include "utils/time.h"
#include "utils/timer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/core.h
 * @brief General tangle-accelerator core functions
 *
 * tangle-accelerator core functions provide major IOTA usage with
 * `iota.c/cclient` and can be wrapped into vary APIs.
 * The arguments and return data structure are specified in different
 * requests.
 *
 */

/**
 * @brief Send transfer to tangle.
 *
 * Build the transfer bundle from request and broadcast to the tangle. Input
 * fields include address, value, tag, and message. This API would also try to
 * find the transactions after bundle sent.
 *
 * @param info [in] Tangle-accelerator configuration variables
 * @param[in] iconf IOTA API parameter configurations
 * @param service [in] IOTA full node end point service
 * @param[in] cache redis configuration variables
 * @param[in] req Request containing address value, message, tag in
 *                ta_send_transfer_req_t
 * @param[out] res Result containing transaction hash in ta_send_transfer_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_transfer(const ta_config_t* const info, const iota_config_t* const iconf,
                          const iota_client_service_t* const service, const ta_cache_t* const cache,
                          const ta_send_transfer_req_t* const req, ta_send_transfer_res_t* res);

/**
 * @brief Send trytes to tangle.
 *
 * Get trunk and branch in `cclient_get_txn_to_approve`, create
 * bundle and do PoW in `ta_attach_to_tangle` and store and broadcast
 * transaction to tangle.
 *
 * @param[in] info Tangle-accelerator configuration variables
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA full node end point service
 * @param[in, out] trytes Trytes that will be attached to tangle. The output trytes are the ones with completed PoW and
 * Tangle broadcasting, and broadcast to Tangle.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_trytes(const ta_config_t* const info, const iota_config_t* const iconf,
                        const iota_client_service_t* const service, hash8019_array_p trytes);

/**
 * @brief Return list of transaction hash with given tag.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction hash in ta_find_transactions_by_tag_res_t.
 *
 * @param[in] service IOTA full node end point service
 * @param[in] req tag in trytes
 * @param[out] res Result containing a list of transaction hash in
 *             ta_find_transactions_by_tag_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transactions_by_tag(const iota_client_service_t* const service, const char* const req,
                                     ta_find_transactions_by_tag_res_t* const res);

/**
 * @brief Return list of transaction object with given tag.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction objects in ta_find_transactions_obj_res_t.
 *
 * @param[in] service IOTA full node end point service
 * @param[in] req find_transactions_req_t object which contains tags
 * @param[out] res Result containing list of transaction objects in
 *                 transaction_array_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transactions_obj_by_tag(const iota_client_service_t* const service,
                                         const find_transactions_req_t* const req, transaction_array_t* res);

/**
 * @brief Get transaction objects with transaction hashes when there are multiple transaction hashes are waiting for
 * fetching. This function will fetch a transaction object with a transaction hash each time.
 *
 * @param[in] service IOTA full node end point service
 * @param[in] tx_queries Given find_transactions_req_t with transaction hashes
 * @param[out] tx_objs Return transaction objects
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_txn_objects_with_txn_hash(const iota_client_service_t* const service,
                                          find_transactions_req_t* tx_queries, transaction_array_t* tx_objs);

/**
 * @brief Return transaction object with given transaction hashes.
 *
 * Explore transaction hash information with given transaction hashes. This would
 * return whole transaction object details in transaction_array_t
 * instead of raw trytes, includes address, value, timestamp, mwm, nonce...
 *
 * @param[in] service IOTA full node end point service
 * @param[in] req Given transaction hashes
 * @param[out] res Result containing transaction objects in transaction_array_t.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transaction_objects(const iota_client_service_t* const service,
                                     const ta_find_transaction_objects_req_t* const req, transaction_array_t* res);

/**
 * @brief Return bundle object with given bundle hash.
 *
 * Explore transaction hash information with given bundle hash. This would
 * return only one bundle objects in bundle_transactions_t instead of all
 * transactions like reattached ones.
 *
 * @param[in] service IOTA full node end point service
 * @param[in] bundle_hash bundle hash in trytes
 * @param[out] bundle Result containing bundle object in bundle_transactions_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_bundle(const iota_client_service_t* const service, tryte_t const* const bundle_hash,
                       bundle_transactions_t* const bundle);

/**
 * @brief Send bundle object.
 *
 * Send the unpacked bundle which contains transactions. MAM functions should
 * send message with this function.
 *
 * @param[in] info Tangle-accelerator configuration variables
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA full node end point service
 * @param[in] bundle bundle object to send
 * @param[out] bundle Result containing bundle object in bundle_transactions_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_bundle(const ta_config_t* const info, const iota_config_t* const iconf,
                        const iota_client_service_t* const service, bundle_transactions_t* const bundle);

/**
 * @brief Get the bundle that contains assigned address
 *
 * We can get a bundle with any address in the bundle. Moreover, because the channel ID in MAM is actually the address
 * of a transaction, we can use this function to search which bundle contains the message transaction we want to fetch.
 *
 * @param[in] service IOTA full node end point service
 * @param[in] addr searched address in tryte_t
 * @param[in] bundle_array a bundle array object that will contain the MAM transactions
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_bundles_by_addr(const iota_client_service_t* const service, tryte_t const* const addr,
                                bundle_array_t* bundle_array);

/**
 * @brief Get current connection status. The status will be responded with return value.
 *
 * We would check out the connection status with IOTA core API getNodeInfo. At the first step, we would check whether
 * tangle-accelerator can connect to the IOTA full node which is assigned in iota_client_service_t object. If the
 * tangle-accelerator connects to the IOTA full node, the next step we are going to check out whether the connected node
 * has synchronized to the latest milestone. The mentioned two errors above would trigger tangle-accelerator connect to
 * another IOTA full node on node priority host list.
 *
 * @param[in] service IOTA full node end point service
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_node_status(const iota_client_service_t* const service);

/**
 * @brief Push failed transactions in raw trytes into transaction buffer
 *
 * Given raw trytes array would be pushed into buffer. An UUID will be returned for client to fetch the information of
 * their request. The UUIDs are stored in a list, so once reaching the capacity of the buffer, buffered transactions can
 * be popped from the buffer.
 *
 * @param[in] cache Redis configuration variables
 * @param[in] raw_txn_flex_trit_array Raw transaction trytes array in flex_trit_t type
 * @param[out] uuid Returned UUID for fetching transaction status and information
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t push_txn_to_buffer(const ta_cache_t* const cache, hash8019_array_p raw_txn_flex_trit_array, char* uuid);

/**
 * @brief Return the transaction object status according to the given UUID
 *
 * If the given UUID points to a sent transaction, then `ta_fetch_buffered_request_status` will return the
 * content of the transaction object. If the transaction have been sent yet, then return unsent. If tangle-accelerator
 * can't find the UUID in redis then it will return no_exist. In the current implementation, we used Redis to buffer all
 * transactions.
 *
 * @param[in] cache redis configuration variables
 * @param[in] uuid Given UUID
 * @param[out] res ta_fetch_buffered_request_status_res_t contains the transaction object and status
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_fetch_txn_with_uuid(const ta_cache_t* const cache, const char* const uuid,
                                ta_fetch_buffered_request_status_res_t* res);

/**
 * @brief Return the MAM request status according to the given UUID
 *
 * @param[in] cache redis configuration variables
 * @param[in] uuid Given UUID
 * @param[out] res ta_recv_mam_res_t object contains the transaction object and status
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_fetch_mam_with_uuid(const ta_cache_t* const cache, const char* const uuid,
                                ta_fetch_buffered_request_status_res_t* res);

#ifdef __cplusplus
}
#endif

#endif  // CORE_CORE_H
