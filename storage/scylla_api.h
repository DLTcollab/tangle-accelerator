/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef TA_SCYLLA_API_H_
#define TA_SCYLLA_API_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "accelerator/errors.h"
#include "cassandra.h"
#include "common/model/transaction.h"
#include "utils/containers/hash/hash243_queue.h"
#include "utils/logger_helper.h"

typedef struct scylla_iota_transaction_s scylla_iota_transaction_t;

/**
 * @brief malloc memory space to *obj
 *
 * @param[in] obj pointer to pointer to scylla_iota_transaction_t
 *
 * @return
 * - SC_OK on success
 * - SC_STORAGE_OOM on error
 */
status_t new_scylla_iota_transaction(scylla_iota_transaction_t** obj);

/**
 * @brief release memory space to *obj
 *
 * @param[in] obj pointer to pointer to scylla_iota_transaction_t
 */
void free_scylla_iota_transaction(scylla_iota_transaction_t** obj);

/**
 * @brief set hash in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] hash input hash to be set to member of scylla_iota_transaction_t
 * @param[in] length length of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_hash(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length);

/**
 * @brief set bundle in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] hash input hash to be set to member of scylla_iota_transaction_t
 * @param[in] length length of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_bundle(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length);

/**
 * @brief set address in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] hash input hash to be set to member of scylla_iota_transaction_t
 * @param[in] length length of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_address(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length);

/**
 * @brief set trunk in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] hash input hash to be set to member of scylla_iota_transaction_t
 * @param[in] length length of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_trunk(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length);

/**
 * @brief set branch in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] hash input hash to be set to member of scylla_iota_transaction_t
 * @param[in] length length of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_branch(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length);

/**
 * @brief set message in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] hash input hash to be set to member of scylla_iota_transaction_t
 * @param[in] length length of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_message(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length);

/**
 * @brief set value of iota token in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] value input value to be set to member of scylla_iota_transaction_t
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_value(scylla_iota_transaction_t* obj, int64_t value);

/**
 * @brief set timestamp in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 * @param[in] timestamp input timestamp to be set to member of scylla_iota_transaction_t
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t set_transaction_timestamp(scylla_iota_transaction_t* obj, int64_t timestamp);

/**
 * @brief return hash in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - hash on success
 * - NULL on error
 */
cass_byte_t* ret_transaction_hash(scylla_iota_transaction_t* obj);

/**
 * @brief return hash in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - hash on success
 * - NULL on error
 */
cass_byte_t* ret_transaction_bundle(scylla_iota_transaction_t* obj);

/**
 * @brief return address in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - address on success
 * - NULL on error
 */
cass_byte_t* ret_transaction_address(scylla_iota_transaction_t* obj);

/**
 * @brief return trunk in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - trunk on success
 * - NULL on error
 */
cass_byte_t* ret_transaction_trunk(scylla_iota_transaction_t* obj);

/**
 * @brief return branch in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - branch on success
 * - NULL on error
 */
cass_byte_t* ret_transaction_branch(scylla_iota_transaction_t* obj);

/**
 * @brief return message in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - message on success
 * - NULL on error
 */
cass_byte_t* ret_transaction_message(scylla_iota_transaction_t* obj);

/**
 * @brief return value in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - value on success
 * - NULL on error
 */
int64_t ret_transaction_value(scylla_iota_transaction_t* obj);

/**
 * @brief return timestamp in scylla_iota_transaction_t
 *
 * @param[in] obj pointer to scylla_iota_transaction_t
 *
 * @return
 * - timestamp on success
 * - NULL on error
 */
int64_t ret_transaction_timestamp(scylla_iota_transaction_t* obj);

/**
 * Initialize logger
 */
void scylla_api_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int scylla_api_logger_release();

/**
 * @brief connect to Scylla node and create table
 *
 * @param[out] cluster Scylla node cluster
 * @param[out] session Scylla cluster session
 * @param[in] hosts Scylla node ip
 * @param[in] is_need_create_table true : create, false : not to create
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t init_scylla(CassCluster** cluster, CassSession* session, char* hosts, bool is_need_create_table);

/**
 * @brief insert transactions into bundle table
 *
 * @param[in] session Scylla cluster session
 * @param[in] transaction input transactions data
 * @param[in] tran_num input transactions number
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t insert_transaction_into_bundleTable(CassSession* session, scylla_iota_transaction_t* transaction,
                                             size_t trans_num);

/**
 * @brief insert transactions into edge table
 *
 * @param[in] session Scylla cluster session
 * @param[in] transaction input transactions data
 * @param[in] tran_num input transactions number
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t insert_transaction_into_edgeTable(CassSession* session, scylla_iota_transaction_t* transaction,
                                           size_t trans_num);

/**
 * @brief get bundles from edge table by edge
 *
 * @param[in] session Scylla cluster session
 * @param[in] edge primary key for select bundles
 * @param[in] column_name the column we want to select
 * @param[out] res_queue response transactions queue
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t get_column_from_edgeTable(CassSession* session, hash243_queue_t* res_queue, cass_byte_t* edge,
                                   const char* column_name);

/**
 * @brief get transactions by bundles, addresses or get approves
 *
 * @param[in] session Scylla cluster session
 * @param[in] bundles query bundles queue
 * @param[in] bundles query addresses queue
 * @param[in] bundles query approves queue
 * @param[out] res_queue response transactions queue
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t get_transactions(CassSession* session, hash243_queue_t* res_queue, hash243_queue_t bundles,
                          hash243_queue_t addresses, hash243_queue_t approves);

#ifdef __cplusplus
}
#endif

#endif  // TA_SCYLLA_API_H_
