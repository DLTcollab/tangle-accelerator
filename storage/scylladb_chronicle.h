/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef STORAGE_SCYLLADB_CHRONICLE_H_
#define STORAGE_SCYLLADB_CHRONICLE_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "cclient/request/find_transactions.h"
#include "cclient/request/get_inclusion_states.h"
#include "cclient/request/get_trytes.h"
#include "cclient/response/find_transactions.h"
#include "cclient/response/get_inclusion_states.h"
#include "cclient/response/get_trytes.h"
#include "common/model/transaction.h"
#include "scylladb_client.h"
#include "utils/containers/hash/hash8019_queue.h"

/**
 * @file storage/scylladb_chronicle.h
 * @brief c implement ch.
 */

/**
 * @brief connect to ScyllaDB node and create table
 *
 * @param[in] service ScyllaDB db client service
 * @param[in] need_truncate true : clear all data, false : keep old data
 * @param[in] keyspace_name keyspace name the session should use
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_chronicle_keyspace_init(const db_client_service_t* service, bool need_truncate, const char* keyspace_name);

/**
 * @brief insert transactions into bundle table
 *
 * @param[in] service ScyllaDB client service
 * @param[in] trytes input transaction trytes
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_chronicle_insert_transaction(const db_client_service_t* service, const tryte_t* hash,
                                         const tryte_t* trytes);

/**
 * @brief get transaction hash by given bundle
 *
 * @param[in] service ScyllaDB client service
 * @param[out] res result hash queue for transaction hash
 * @param[in] bundle target bundle hash
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_transactions_by_bundle(const db_client_service_t* service, hash243_queue_t* res,
                                       const flex_trit_t* bundle);

/**
 * @brief get transaction hash by given address
 *
 * @param[in] service ScyllaDB client service
 * @param[out] res result hash queue for transaction hash
 * @param[in] address target address hash
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_transactions_by_address(const db_client_service_t* service, hash243_queue_t* res,
                                        const flex_trit_t* address);

/**
 * @brief get transaction hash by given tag
 *
 * @param[in] service ScyllaDB client service
 * @param[out] res result hash queue for transaction hash
 * @param[in] tag target tag
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_transactions_by_tag(const db_client_service_t* service, hash243_queue_t* res, const flex_trit_t* tag);

/**
 * @brief get transaction approvees
 *
 * @param[in] service ScyllaDB client service
 * @param[out] res result hash queue for approvees
 * @param[in] hash target transaction hash
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_approvee(const db_client_service_t* service, hash243_queue_t* res, const flex_trit_t* hash);

/**
 * @brief get transaction trytes
 *
 * @param[in] service ScyllaDB client service
 * @param[out] res result hash queue for trytes
 * @param[in] hash target transaction hash
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_trytes(const db_client_service_t* service, hash8019_queue_t* res, const flex_trit_t* hash);

/**
 * @brief Finds transactions that contain the given values in their transaction fields. The parameters define
 *        the transaction fields to search for, including bundles, addresses, tags, and approvees
 * @param[in] service ScyllaDB client service
 * @param[in] req request transaction fields
 * @param[out] res result of transaction hashes
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_find_transactions(const db_client_service_t* service, const find_transactions_req_t* req,
                                     find_transactions_res_t* const res);

/**
 * @brief Finds transaction objects by given hashes
 * @param[in] service ScyllaDB client service
 * @param[in] tx_hashes request transaction hashes
 * @param[out] out_tx_objs result of transaction objects
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_get_transaction_objects(const db_client_service_t* service, const get_trytes_req_t* tx_hashes,
                                           transaction_array_t* out_tx_objs);

/**
 * @brief Finds transaction trytes by given hashes
 * @param[in] service ScyllaDB client service
 * @param[in] req request transaction hashes
 * @param[out] res result of trytes
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_get_trytes(const db_client_service_t* service, const get_trytes_req_t* req, get_trytes_res_t* res);

/**
 * @brief get inclustion status by given hashes
 * @param[in] service ScyllaDB client service
 * @param[in] req request transaction hashes
 * @param[out] res result of trytes
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_get_inclusion_states(const db_client_service_t* service, get_inclusion_states_req_t const* const req,
                                        get_inclusion_states_res_t* res);

#ifdef __cplusplus
}
#endif

#endif  // STORAGE_SCYLLADB_CHRONICLE_H_