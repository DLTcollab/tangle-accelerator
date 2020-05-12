/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef STORAGE_SCYLLADB_CLIENT_H_
#define STORAGE_SCYLLADB_CLIENT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "scylladb_utils.h"

/**
 * @file storage/scylladb_client.h
 * @brief ScyllaDB client service for connection and executing queries.
 */

#define DB_UUID_STRING_LENGTH CASS_UUID_STRING_LENGTH

/*
 * DB_USAGE_NULL is for non-preserved usage and user-defined keyspace.
 * Call db_init_identity_keyspace to give the keyspace name.
 * This method could be enhanced by supporting user-defined keyspace name when
 * specific the preserved usage.
 */
typedef enum { DB_USAGE_REATTACH = 0, DB_USAGE_PERMANODE, DB_USAGE_NULL, NUM_DB_USAGE } db_client_usage_t;
typedef struct {
  CassCluster* cluster;
  CassSession* session;
  char* host;
  /**< CassUuidGen is a UUID generator object, which is thread-safe to generate UUIDs */
  CassUuidGen* uuid_gen;
  bool enabled; /**< switch of db connection */
} db_client_service_t;

/**
 * @brief init ScyllaDB client serivce and connect to specific cluster
 *
 * @param[out] service ScyllaDB client service
 * @param[in] usage specfic usage for db client serivce
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_service_init(db_client_service_t* service, db_client_usage_t usage);

/**
 * @brief free ScyllaDB client serivce
 *
 * @param[in] service ScyllaDB client service
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_service_free(db_client_service_t* service);

#ifdef __cplusplus
}
#endif

#endif  // STORAGE_SCYLLADB_CLIENT_H_
