/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_COMMON_CORE_H_
#define ACCELERATOR_COMMON_CORE_H_

#include "accelerator/config.h"
#include "common/model/transfer.h"
#include "request/request.h"
#include "response/response.h"
#include "utils/time.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file common_core.h
 * @brief General tangle-accelerator core functions
 *
 * tangle-accelerator core functions provide major IOTA usage with
 * `entangled/cclient` and can be wrapped into vary APIs.
 * The arguments and return data structure are specified in different
 * requests.
 *
 * @example test_common.cc
 */

/**
 * Initialize logger
 */
void cc_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int cc_logger_release();

/**
 * @brief Get trunk and branch transactions
 *
 * Get a tips pair as trunk/branch transactions for transaction construction.
 * The result is a 243 long flex trits hash stack.
 *
 * @param[in] service IRI node end point service
 * @param[in] depth Depth of get transaction to approve
 * @param[out] res Result containing a tips pair in ta_get_tips_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cclient_get_txn_to_approve(const iota_client_service_t* const service, uint8_t const depth,
                                    ta_get_tips_res_t* res);

/**
 * @brief Generate an unused address.
 *
 * Generate and return an unused address from the seed. An unused address means
 * the address does not have any transaction with it yet.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[out] res Result containing an unused address in
 * ta_generate_address_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_generate_address(const iota_config_t* const iconf, const iota_client_service_t* const service,
                             ta_generate_address_res_t* res);

/**
 * @brief Send transfer to tangle.
 *
 * Build the transfer bundle from request and broadcast to the tangle. Input
 * fields include address, value, tag, and message. This API would also try to
 * find the transactions after bundle sent.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[in] req Request containing address value, message, tag in
 *                ta_send_transfer_req_t
 * @param[out] res Result containing transaction hash in ta_send_transfer_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_transfer(const iota_config_t* const iconf, const iota_client_service_t* const service,
                          const ta_send_transfer_req_t* const req, ta_send_transfer_res_t* res);

/**
 * @brief Send trytes to tangle.
 *
 * Get trunk and branch in `cclient_get_txn_to_approve`, create
 * bundle and do PoW in `ta_attach_to_tangle` and store and broadcast
 * transaction to tangle.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[in] trytes Trytes that will be attached to tangle
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_trytes(const iota_config_t* const iconf, const iota_client_service_t* const service,
                        hash8019_array_p trytes);

/**
 * @brief Return list of transaction hash with given tag.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction hash in ta_find_transactions_by_tag_res_t.
 *
 * @param[in] service IRI node end point service
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
 * @param[in] service IRI node end point service
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
 * @brief Return transaction object with given transaction hashes.
 *
 * Explore transaction hash information with given transaction hashes. This would
 * return whole transaction object details in transaction_array_t
 * instead of raw trytes, includes address, value, timestamp, mwm, nonce...
 *
 * @param[in] service IRI node end point service
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
 * @param[in] service IRI node end point service
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
 * @param[in] service IRI node end point service
 * @param[in] bundle bundle object to send
 * @param[out] bundle Result containing bundle object in bundle_transactions_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_bundle(const iota_config_t* const iconf, const iota_client_service_t* const service,
                        bundle_transactions_t* const bundle);

/**
 * @brief Get the bundle that contains assigned address
 *
 * We can get a bundle with any address in the bundle. Moreover, because the channel ID in MAM is actually the address
 * of a transaction, we can use this function to search which bundle contains the message transaction we want to fetch.
 *
 * @param[in] service IRI node end point service
 * @param[in] addr searched address in tryte_t
 * @param[in] bundle pointer of bundle object that will contain the MAM transacitons
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_bundle_by_addr(const iota_client_service_t* const service, tryte_t const* const addr,
                               bundle_transactions_t* bundle);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_COMMON_CORE_H_
