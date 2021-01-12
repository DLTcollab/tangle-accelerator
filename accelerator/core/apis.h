/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_APIS_H_
#define CORE_APIS_H_

#include "accelerator/core/core.h"
#include "accelerator/core/mam_core.h"
#include "serializer/serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/apis.h
 * @brief General tangle-accelerator APIs
 *
 * tangle-accelerator APIs provide major IOTA APIs wrapper for public usage.
 * The arguments and return strings are all in json format. There can be
 * different server or protocol integration with these APIs.
 */

/**
 * @brief Dump tangle accelerator information.
 *
 * @param[in] info Tangle-accelerator configuration variables
 * @param[in] tangle iota configuration variables
 * @param[in] cache redis configuration variables
 * @param[out] json_result Result containing tangle accelerator information in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_ta_info(ta_config_t* const info, iota_config_t* const tangle, ta_cache_t* const cache,
                         char** json_result);

/**
 * @brief Receive MAM messages.
 *
 * Receive a MAM message from given channel id or bundle hash.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA node service
 * @param[in] obj Input data in JSON
 * @param[out] json_result Fetched MAM message in JSON format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_recv_mam_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                              const char* const obj, char** json_result);

/**
 * @brief Send a MAM message with given Payload.
 *
 * Send a MAM message from given request format.
 * There is no need to decode the ascii payload to tryte, since the
 * api_send_mam_message() will take this job.
 *
 * @param[in] info Tangle-accelerator configuration variables
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA node service
 * @param[in] obj Input data in JSON
 * @param[out] json_result Result containing channel id, message id and bundle hash
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_send_mam_message(const ta_cache_t* const cache, char const* const obj, char** json_result);

/**
 * @brief Send transfer to tangle.
 *
 * Build the transfer bundle from request and broadcast to the tangle. Input
 * fields include address, value, tag, and message. This API would also try to
 * find the transactions after bundle sent.
 *
 * @param[in] core Pointer of Tangle-accelerator core configuration structure
 * @param[in] iota_service IOTA node service
 * @param[in] obj Input data in JSON
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_send_transfer(const ta_core_t* const core, const iota_client_service_t* iota_service,
                           const char* const obj, char** json_result);

/**
 * @brief Return transaction object with given single transaction hash.
 *
 * Explore transaction hash information with given single transaction hash. This would
 * return whole transaction object details in json format instead of raw trytes.
 *
 * @param[in] service IOTA node service
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
 * @param[in] service IOTA node service
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
 * @param[in] service IOTA node service
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
 * @param[in] service IOTA node service
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
 * @param[in] info Tangle-accelerator configuration variables
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA node service
 * @param[in] obj trytes to attach, store and broadcast in json array
 * @param[out] json_result Result containing list of attached transaction hashes
 * in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_send_trytes(const ta_config_t* const info, const iota_config_t* const iconf,
                         const iota_client_service_t* const service, const char* const obj, char** json_result);

/**
 * @brief Check the connection status between tangle-accelerator and IOTA full node.
 *
 * @param[in] service IOTA node service
 * @param[out] json_result Result containing the current connection status.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_node_status(const iota_client_service_t* const service, char** json_result);

/**
 * @brief Fetch buffered request information with UUID.
 *
 * @param[in] cache Redis configuration variables
 * @param[in] uuid Requesting UUID
 * @param[out] json_result Result containing the current connection status.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_fetch_buffered_request_status(const ta_cache_t* const cache, const char* const uuid, char** json_result);

/**
 * @brief Register user identity with MAM channel seed
 *
 * @param[in] cache Redis configuration variables
 * @param[in] obj Request in JSON format
 * @param[out] json_result Result contains the user id.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_register_mam_channel(const ta_cache_t* const cache, const char* const obj, char** json_result);

#ifdef DB_ENABLE
/**
 * @brief Return transaction object with given single identity number.
 *
 * Explore transaction hash information with given single identity number. This would
 * return whole transaction object details in json format instead of raw trytes.
 *
 * @param[in] iota_service IOTA node service
 * @param[in] db_service db client service
 * @param[in] obj identity number
 * @param[out] json_result Result containing the only one transaction object in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_find_transactions_by_id(const iota_client_service_t* const iota_service,
                                     const db_client_service_t* const db_service, const char* const obj,
                                     char** json_result);

/**
 * @brief Return db identity object with given single transaction hash.
 *
 * @param[in] db_service db client service
 * @param[in] obj transaction hash
 * @param[out] json_result Result containing the only one db identity object in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_identity_info_by_hash(const db_client_service_t* const db_service, const char* const obj,
                                       char** json_result);

/**
 * @brief Return db identity object with given single transaction id.
 *
 * @param[in] db_service db client service
 * @param[in] obj transaction id
 * @param[out] json_result Result containing the only one db identity object in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_identity_info_by_id(const db_client_service_t* const db_service, const char* const obj,
                                     char** json_result);
#endif

#ifdef __cplusplus
}
#endif

#endif  // CORE_APIS_H_
