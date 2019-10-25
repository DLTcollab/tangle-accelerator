/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_PROXY_APIS_H_
#define ACCELERATOR_PROXY_APIS_H_

#include "accelerator/errors.h"
#include "cclient/api/core/core_api.h"
#include "cclient/request/requests.h"
#include "cclient/response/responses.h"
#include "utils/logger.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file proxy_apis.h
 * @brief Implement Proxy APIs
 *
 * tangle-accelerator provides major IRI Proxy APIs wrapper.
 * The arguments and return strings are all in json format. There can be
 * different server or protocol integration with these APIs.
 */

/**
 * Initialize logger
 */
void proxy_apis_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int proxy_apis_logger_release();

/**
 * Initialize lock
 *
 * @return
 * - zero on success
 * - SC_CONF_LOCK_INIT on error
 */
status_t proxy_apis_lock_init();

/**
 * Destroy lock
 *
 * @return
 * - zero on success
 * - SC_CONF_LOCK_DESTROY on error
 */
status_t proxy_apis_lock_destroy();

/**
 * @brief Proxy API of checkConsistency
 *
 * @param[in] service IRI node end point service
 * @param[in] obj Transaction hashes to check
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_check_consistency(const iota_client_service_t* const service, const char* const obj, char** json_result);

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
 * @brief Proxy API of getBalances
 *
 * @param[in] service IRI node end point service
 * @param[in] obj Addresses, threshold or tips
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_balances(const iota_client_service_t* const service, const char* const obj, char** json_result);

/**
 * @brief Proxy API of getInclusionStates
 *
 * @param[in] service IRI node end point service
 * @param[in] obj Transactions or tips
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_inclusion_states(const iota_client_service_t* const service, const char* const obj,
                                  char** json_result);

/**
 * @brief Proxy API of getNodeInfo
 *
 * @param[in] service IRI node end point service
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_node_info(const iota_client_service_t* const service, char** json_result);

/**
 * @brief Proxy API of getTrytes
 *
 * @param[in] service IRI node end point service
 * @param[in] obj Transaction hashes
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_get_trytes(const iota_client_service_t* const service, const char* const obj, char** json_result);

/**
 * @brief Proxy API of removeNeighbors
 *
 * @param[in] service IRI node end point service
 * @param[in] obj Strings of neighbor URIs to remove
 * @param[out] json_result Result containing transaction objects in json format
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t api_remove_neighbors(const iota_client_service_t* const service, const char* const obj, char** json_result);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_PROXY_APIS_H_
