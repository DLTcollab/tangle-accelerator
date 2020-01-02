/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef TA_SCYLLADB_CLIENT_H_
#define TA_SCYLLADB_CLIENT_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "scylladb_utils.h"

#define DB_UUID_STRING_LENGTH CASS_UUID_STRING_LENGTH
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
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_client_service_init(db_client_service_t* service);

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

#endif  // TA_SCYLLADB_CLIENT_H_
