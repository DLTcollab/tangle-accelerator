/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_APIS_H_
#define ACCELERATOR_APIS_H_

#include "accelerator/common_core.h"
#include "common/trinary/trit_tryte.h"
#include "mam/api/api.h"
#include "mam/mam/mam_channel_t_set.h"
#include "serializer/serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file apis.h
 * @brief General tangle-accelerator APIs
 *
 * tangle-accelerator APIs provide major IOTA APIs wrapper for public usage.
 * The arguments and return strings are all in json format. There can be
 * different server or protocol integration with these APIs.
 */

/**
 * Initialize logger
 */
void apis_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int apis_logger_release();

/**
 * @brief Dump tangle accelerator information.
 *
 * @param[out] json_result Result containing tangle accelerator information in json format
 * @param info[in] Tangle-accelerator configuration variables
 * @param tangle[in] iota configuration variables
 * @param cache[in] redis configuration variables
 * @param service[in] IRI connection configuration variables
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_ta_info(char** json_result, ta_config_t* const info, iota_config_t* const tangle,
                         ta_cache_t* const cache, iota_client_service_t* const service);

/**
 * Initialize lock
 *
 * @return
 * - zero on success
 * - SC_CONF_LOCK_INIT on error
 */
status_t apis_lock_init();

/**
 * Destroy lock
 *
 * @return
 * - zero on success
 * - SC_CONF_LOCK_DESTROY on error
 */
status_t apis_lock_destroy();

/**
 * @brief Generate an unused address.
 *
 * Generate and return an unused address from the seed. An unused address means
 * the address does not have any transaction with it yet.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing an unused address in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_generate_address(const iota_config_t* const iconf, const iota_client_service_t* const service,
                              char** json_result);

/**
 * @brief Get trunk and branch transactions
 *
 * Get a tips pair as trunk/branch transactions for transaction construction.
 * The result is char array in json format:
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing a tips pair in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_tips_pair(const iota_config_t* const iconf, const iota_client_service_t* const service,
                           char** json_result);

/**
 * @brief Get list of all tips from IRI node.
 *
 * Get list of all tips from IRI node which usually hash thousands of tips in
 * its queue.
 *
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing list of all tips in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_tips(const iota_client_service_t* const service, char** json_result);

/**
 * @brief Receive a MAM message.
 *
 * Receive a MAM message from given bundle hash.
 *
 * @param[in] service IRI node end point service
 * @param[in] chid channel ID string
 * @param[out] json_result Fetched MAM message in JSON format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_receive_mam_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                                 const char* const chid, char** json_result);

/**
 * @brief Send a MAM message with given Payload.
 *
 * Send a MAM message from given Payload(ascii message).
 * There is no need to decode the ascii payload to tryte, since the
 * api_mam_send_message() will take this job.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[in] payload message to send undecoded ascii string.
 * @param[out] json_result Result containing channel id and bundle hash
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_mam_send_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                              char const* const payload, char** json_result);

/**
 * @brief Send transfer to tangle.
 *
 * Build the transfer bundle from request and broadcast to the tangle. Input
 * fields include address, value, tag, and message. This API would also try to
 * find the transactions after bundle sent.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[in] obj Input data in JSON
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_send_transfer(const iota_config_t* const iconf, const iota_client_service_t* const service,
                           const char* const obj, char** json_result);

/**
 * @brief Return transaction hashes with given information such as bundle hashes, addresses, tags, or approvees.
 *
 * Explore transaction hash with given transaction related information. This would
 * return a list of transaction hashes in json format.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj bundle hashes, addresses, tags, or approvees.
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transactions(const iota_client_service_t* const service, const char* const obj, char** json_result);

/**
 * @brief Return transaction object with given single transaction hash.
 *
 * Explore transaction hash information with given single transaction hash. This would
 * return whole transaction object details in json format instead of raw trytes.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj transaction hash in trytes
 * @param[out] json_result Result containing the only one transaction object in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transaction_object_single(const iota_client_service_t* const service, const char* const obj,
                                            char** json_result);

/**
 * @brief Return transaction object with given transaction hash.
 *
 * Explore transaction hash information with given transaction hash. This would
 * return whole transaction object details in json format instead of raw trytes.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj transaction hash in trytes
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transaction_objects(const iota_client_service_t* const service, const char* const obj,
                                      char** json_result);

/**
 * @brief Return list of transaction hash with given tag.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction hashes in json format.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj tag in trytes
 * @param[out] json_result Result containing list of transaction hashes in json
 *             format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transactions_by_tag(const iota_client_service_t* const service, const char* const obj,
                                      char** json_result);

/**
 * @brief Return list of transaction objects with given tag.
 *
 * Retreive all transactions that have same given tag. The result is a list of
 * transaction objects in json format.
 *
 * @param[in] service IRI node end point service
 * @param[in] obj tag in trytes
 * @param[out] json_result Result containing list of transaction objects in json
 * format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transactions_obj_by_tag(const iota_client_service_t* const service, const char* const obj,
                                          char** json_result);

/**
 * @brief Attach trytes to Tangle and return transaction hashes
 *
 * Persist trytes locally before sending to network.
 * This allows for reattachments and prevents key reuse if trytes can't
 * be recovered by querying the network after broadcasting.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IRI node end point service
 * @param[in] obj trytes to attach, store and broadcast in json array
 * @param[out] json_result Result containing list of attached transaction hashes
 * in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_send_trytes(const iota_config_t* const iconf, const iota_client_service_t* const service,
                         const char* const obj, char** json_result);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_APIS_H_
