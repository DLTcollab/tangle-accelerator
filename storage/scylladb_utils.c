/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "scylladb_utils.h"
#include <stdio.h>
#include <string.h>

#define SCYLLADB_LOGGER "ScyllaDB"

#define logger_id scylladb_logger_id

void scylladb_logger_init() { logger_id = logger_helper_enable(SCYLLADB_LOGGER, LOGGER_DEBUG, true); }

int scylladb_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

static void print_error(CassFuture* future) {
  const char* message;
  size_t message_length;
  cass_future_error_message(future, &message, &message_length);
  ta_log_error("Error: %.*s\n", (int)message_length, message);
}

CassError execute_query(CassSession* session, const char* query) {
  CassError rc = CASS_OK;
  CassStatement* statement = cass_statement_new(query, 0);
  CassFuture* future = cass_session_execute(session, statement);

  cass_future_wait(future);

  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  }

  cass_future_free(future);
  cass_statement_free(statement);

  return rc;
}

CassError prepare_query(CassSession* session, const char* query, const CassPrepared** prepared) {
  CassError rc = CASS_OK;
  CassFuture* future = NULL;

  future = cass_session_prepare(session, query);
  cass_future_wait(future);

  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  } else {
    *prepared = cass_future_get_prepared(future);
  }

  cass_future_free(future);

  return rc;
}

CassError execute_statement(CassSession* session, CassStatement* statement) {
  CassError rc = CASS_OK;
  CassFuture* future = NULL;
  future = cass_session_execute(session, statement);

  cass_future_wait(future);

  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  }

  cass_future_free(future);
  return rc;
}

status_t make_query(char** result, const char* head_desc, const char* position, const char* left_desc) {
  if (head_desc == NULL || position == NULL || left_desc == NULL) {
    ta_log_error("NULL pointer to CQL query\n");
    return SC_NULL;
  }
  size_t head_len = strlen(head_desc);
  size_t pos_len = strlen(position);
  size_t left_len = strlen(left_desc);
  size_t result_len = head_len + pos_len + left_len + 1;
  *result = malloc(result_len * sizeof(char));
  if (*result == NULL) {
    ta_log_error("%s\n", "SC_OOM");
    return SC_OOM;
  }
  memcpy(*result, head_desc, head_len);
  memcpy(*result + head_len, position, pos_len);
  memcpy(*result + head_len + pos_len, left_desc, left_len);
  (*result)[result_len - 1] = 0;

  return SC_OK;
}

status_t db_truncate_table(CassSession* session, const char* table_name) {
  status_t ret = SC_OK;
  char* query = NULL;
  ret = make_query(&query, "TRUNCATE TABLE ", table_name, "");
  if (ret != SC_OK) {
    ta_log_error("Fail to make truncate query\n");
    return ret;
  }
  if (execute_query(session, query) != CASS_OK) {
    ta_log_error("Fail to truncate table:  %s\n", table_name);
    ret = SC_STORAGE_CASSANDRA_QUERY_FAIL;
  }
  free(query);
  return ret;
}

status_t create_keyspace(CassSession* session, const char* keyspace_name) {
  status_t ret = SC_OK;
  char* create_query = NULL;
  ret = make_query(&create_query, "CREATE KEYSPACE IF NOT EXISTS ", keyspace_name,
                   " WITH replication = {"
                   "'class': 'SimpleStrategy', 'replication_factor': '2'};");
  if (ret != SC_OK) {
    ta_log_error("%s\n", "make Create keyspace query fail");
    return ret;
  }
  if (execute_query(session, create_query) != CASS_OK) {
    ta_log_error("Create keyspace %s fail\n", keyspace_name);
    ret = SC_STORAGE_CASSANDRA_QUERY_FAIL;
  }

  free(create_query);
  return ret;
}
