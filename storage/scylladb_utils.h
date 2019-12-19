/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef TA_SCYLLADB_UTILS_H_
#define TA_SCYLLADB_UTILS_H_
#ifdef __cplusplus
extern "C" {
#endif
#include <stdlib.h>
#include "accelerator/errors.h"
#include "cassandra.h"
#include "utils/logger.h"

logger_id_t scylladb_logger_id;

/**
 * @brief execute CQL query
 *
 * @param[in] session used to execute queries and maintains cluster state
 * @param[in] query executing query
 * @return
 * - CASS_OK on success
 * - non-zero on error
 */
CassError execute_query(CassSession* session, const char* query);

/**
 * @brief prepare CQL query
 *
 * @param[in] session used to execute queries and maintains cluster state
 * @param[in] query pararing query
 * @return
 * - CASS_OK on success
 * - non-zero on error
 */
CassError prepare_query(CassSession* session, const char* query, const CassPrepared** prepared);

/**
 * @brief execute CQL statement
 *
 * @param[in] session used to execute queries and maintains cluster state
 * @param[in] statement executing statement
 * @return
 * - CASS_OK on success
 * - non-zero on error
 */
CassError execute_statement(CassSession* session, CassStatement* statement);

/**
 * @brief combine three strings to a CQL query
 *
 * @param[in] head_desc first part of query
 * @param[in] position keyspace name or table name
 * @param[in] left_desc left part of query
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t make_query(char** result, const char* head_desc, const char* position, const char* left_desc);

/**
 * @brief create ScyllaDB keyspace
 *
 * @param[in] session used to execute queries and maintains cluster state
 * @param[in] keyspace_name name of keyspace to be created
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t create_keyspace(CassSession* session, const char* keyspace_name);

/**
 * Initialize logger
 */
void scylladb_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int scylladb_logger_release();

#ifdef __cplusplus
}
#endif

#endif  // TA_SCYLLADB_UTILS_H_