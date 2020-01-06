/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef STORAGE_SCYLLADB_IDENTITY_H_
#define STORAGE_SCYLLADB_IDENTITY_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "scylladb_client.h"
#include "utarray.h"

/**
 * @file scylladb_identity.h
 */

typedef struct db_identity_s db_identity_t;
typedef UT_array db_identity_array_t;

typedef enum { PENDING_TXN = 0, INSERTING_TXN, CONFIRMED_TXN, NUM_OF_TXN_STATUS } db_txn_status_t;

/**
 * Allocate memory of db_identity_array_t
 */
db_identity_array_t* db_identity_array_new();

/**
 * @brief The identity array iterator.
 */
#define IDENTITY_TABLE_ARRAY_FOREACH(identities, itr)                \
  for (itr = (db_identity_t*)utarray_front(identities); itr != NULL; \
       itr = (db_identity_t*)utarray_next(identities, itr))

/**
 * Free memory of db_identity_array_t
 */
static inline void db_identity_array_free(db_identity_array_t** const db_identity_array) {
  if (db_identity_array && *db_identity_array) {
    utarray_free(*db_identity_array);
  }
  *db_identity_array = NULL;
}

/**
 * @brief malloc memory space to *obj
 *
 * @param[in] obj pointer to pointer to db_identity_t
 *
 * @return
 * - SC_OK on success
 * - SC_STORAGE_OOM on error
 */
status_t db_identity_new(db_identity_t** obj);

/**
 * @brief release memory space to *obj
 *
 * @param[in] obj pointer to pointer to db_identity_t to be free
 */
void db_identity_free(db_identity_t** obj);

/**
 * @brief set id in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 * @param[in] in pointer to CassUuid to be set into db_identity_t
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t db_set_identity_uuid(db_identity_t* obj, CassUuid* in);

/**
 * @brief covert and return uuid in db_identity_t to uuid string
 *
 * @param[in] obj pointer to db_identity_t
 * @param[out] res_uuid_string buffer for CassUuid converting uuid string
 * @return
 * - id on success
 * - INT64_MAX on error
 */

status_t db_get_identity_uuid_string(const db_identity_t* obj, char* res_uuid_string);

/**
 * @brief return time(in seconds) elapsed from identity obj update time to current time
 *
 * @param[in] obj pointer to db_identity_t
 *
 * @return
 * - time(in seconds) on success
 * - 0 on error
 */
cass_int64_t db_ret_identity_time_elapsed(db_identity_t* obj);

/**
 * @brief set status in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 * @param[in] status status to be set into db_identity_t
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t db_set_identity_status(db_identity_t* obj, cass_int8_t status);

/**
 * @brief return status in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 *
 * @return
 * - status on success
 * - NULL on error
 */
cass_int8_t db_ret_identity_status(const db_identity_t* obj);

/**
 * @brief set hash in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 * @param[in] hash hash to be set into db_identity_t
 * @param[in] length size of hash
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t db_set_identity_hash(db_identity_t* obj, const cass_byte_t* hash, size_t length);

/**
 * @brief return hash in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 *
 * @return
 * - pointer to hash on success
 * - NULL on error
 */
const cass_byte_t* db_ret_identity_hash(const db_identity_t* obj);

/**
 * @brief generate a new db_identity_t obj with specific hash and status
 *
 * @param[in] service ScyllaDB client service
 * @param[in] hash transaction hash to be set
 * @param[in] status transaction status to be set
 * @param[out] res_uuid_string a buffer to get new generated version 4 uuid string, buffer size must bigger than
 * DB_UUID_STRING_LENGTH
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_insert_tx_into_identity(db_client_service_t* service, const char* hash, db_txn_status_t status,
                                    char* res_uuid_string);

/**
 * @brief connect to ScyllaDB cluster and initialize identity keyspace and table
 *
 * @param[in] service ScyllaDB client service for connection
 * @param[in] need_drop true : drop table, false : keep old table
 * @param[in] keyspace_name keyspace name the session should use
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_init_identity_keyspace(db_client_service_t* service, bool need_drop, const char* keyspace_name);

/**
 * @brief get identity objs with selected status from identity table
 *
 * @param[in] service ScyllaDB client service for connection
 * @param[in] status selected status TXN status
 * @param[out] db_identity_array UT arrray for db_identity_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_identity_objs_by_status(db_client_service_t* service, cass_int8_t status,
                                        db_identity_array_t* db_identity_array);
/**
 * @brief set time(in seconds) in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 * @param[in] ts timestamp to be set into db_identity_t
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t db_set_identity_timestamp(db_identity_t* obj, cass_int64_t ts);

/**
 * @brief return time(in seconds) in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 *
 * @return
 * - time(in seconds) on success
 * - NULL on error
 */
cass_int64_t db_ret_identity_timestamp(const db_identity_t* obj);

/**
 * @brief get identity objs with selected uuid in string format from identity table
 *
 * @param[in] service ScyllaDB client service for connection
 * @param[in] uuid_string selected uuid string
 * @param[out] db_identity_array UT arrray for db_identity_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_identity_objs_by_uuid_string(db_client_service_t* service, const char* uuid_string,
                                             db_identity_array_t* db_identity_array);

/**
 * @brief get identity objs with selected hash from identity table
 *
 * @param[in] service ScyllaDB client service for connection
 * @param[in] hash selected TXN hash
 * @param[out] db_identity_array UT arrray for db_identity_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_get_identity_objs_by_hash(db_client_service_t* service, const cass_byte_t* hash,
                                      db_identity_array_t* db_identity_array);

/**
 * @brief insert db_identity_t into identity table
 *
 * @param[in] service ScyllaDB client service for connection
 * @param[in] obj inserted db_identity_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_insert_identity_table(db_client_service_t* service, db_identity_t* obj);

#ifdef __cplusplus
}
#endif

#endif  // STORAGE_SCYLLADB_IDENTITY_H_