/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "scylla_api.h"

#define SCYLLA_API_LOGGER "scylladb"

static logger_id_t logger_id;

void scylla_api_logger_init() { logger_id = logger_helper_enable(SCYLLA_API_LOGGER, LOGGER_DEBUG, true); }

int scylla_api_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(logger_id, "%s.\n", SCYLLA_API_LOGGER);
    return EXIT_FAILURE;
  }
  return 0;
}

typedef struct select_where_s {
  cass_byte_t* bundle;
  cass_byte_t* address;
} select_where_t;

typedef enum { WITH_BUNDLE = 0, WITH_BUNDLE_AND_ADDRESS, NUM_OF_SELECT_METHODS } select_method_t;

typedef struct query_s {
  const char* query;
} query_t;

struct scylla_iota_transaction_s {
  cass_byte_t bundle[NUM_FLEX_TRITS_BUNDLE];
  cass_byte_t address[NUM_FLEX_TRITS_ADDRESS];
  cass_byte_t hash[NUM_FLEX_TRITS_HASH];
  cass_byte_t message[NUM_FLEX_TRITS_MESSAGE];
  cass_int64_t value;
  cass_int64_t timestamp;
  cass_byte_t trunk[NUM_FLEX_TRITS_TRUNK];
  cass_byte_t branch[NUM_FLEX_TRITS_BRANCH];
};

static const query_t select_query[NUM_OF_SELECT_METHODS] = {
    {"SELECT * FROM bundleTable WHERE bundle = ?"}, {"SELECT * FROM bundleTable WHERE bundle = ? AND address = ?"}};

status_t new_scylla_iota_transaction(scylla_iota_transaction_t** obj) {
  *obj = (scylla_iota_transaction_t*)malloc(sizeof(scylla_iota_transaction_t));
  if (NULL == *obj) {
    ta_log_error("%s\n", "SC_STORAGE_OOM");
    return SC_STORAGE_OOM;
  }
  return SC_OK;
}

void free_scylla_iota_transaction(scylla_iota_transaction_t** obj) {
  free(*obj);
  *obj = NULL;
}

status_t set_transaction_hash(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to set into ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (length != NUM_FLEX_TRITS_HASH) {
    ta_log_error("%s\n", "SC_STORAGE_INVAILD_INPUT");
    return SC_STORAGE_INVAILD_INPUT;
  }
  memcpy(obj->hash, hash, NUM_FLEX_TRITS_HASH);
  return SC_OK;
}

cass_byte_t* ret_transaction_hash(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return NULL;
  }
  return obj->hash;
}

status_t set_transaction_address(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to set into ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (length != NUM_FLEX_TRITS_ADDRESS) {
    ta_log_error("%s\n", "SC_STORAGE_INVAILD_INPUT");
    return SC_STORAGE_INVAILD_INPUT;
  }
  memcpy(obj->address, hash, length);
  return SC_OK;
}

cass_byte_t* ret_transaction_address(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return NULL;
  }
  return obj->address;
}

status_t set_transaction_bundle(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to set into ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (length != NUM_FLEX_TRITS_BUNDLE) {
    ta_log_error("%s\n", "SC_STORAGE_INVAILD_INPUT");
    return SC_STORAGE_INVAILD_INPUT;
  }
  memcpy(obj->bundle, hash, length);
  return SC_OK;
}

cass_byte_t* ret_transaction_bundle(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return NULL;
  }
  return obj->bundle;
}

status_t set_transaction_trunk(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to set into ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (length != NUM_FLEX_TRITS_TRUNK) {
    ta_log_error("%s\n", "SC_STORAGE_INVAILD_INPUT");
    return SC_STORAGE_INVAILD_INPUT;
  }
  memcpy(obj->trunk, hash, length);
  return SC_OK;
}

cass_byte_t* ret_transaction_trunk(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return NULL;
  }
  return obj->trunk;
}

status_t set_transaction_branch(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to set into ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (length != NUM_FLEX_TRITS_BRANCH) {
    ta_log_error("%s\n", "SC_STORAGE_INVAILD_INPUT");
    return SC_STORAGE_INVAILD_INPUT;
  }
  memcpy(obj->branch, hash, length);
  return SC_OK;
}

cass_byte_t* ret_transaction_branch(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return NULL;
  }
  return obj->branch;
}

status_t set_transaction_message(scylla_iota_transaction_t* obj, cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to set into ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  if (length != NUM_FLEX_TRITS_MESSAGE) {
    ta_log_error("%s\n", "SC_STORAGE_INVAILD_INPUT");
    return SC_STORAGE_INVAILD_INPUT;
  }

  memcpy(obj->message, hash, length);

  return SC_OK;
}

cass_byte_t* ret_transaction_message(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return NULL;
  }
  return obj->message;
}

status_t set_transaction_value(scylla_iota_transaction_t* obj, int64_t value) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  obj->value = value;
  return SC_OK;
}

int64_t ret_transaction_value(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return 0;
  }
  return obj->value;
}

status_t set_transaction_timestamp(scylla_iota_transaction_t* obj, int64_t timestamp) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }
  obj->timestamp = timestamp;
  return SC_OK;
}

int64_t ret_transaction_timestamp(scylla_iota_transaction_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return 0;
  }
  return obj->timestamp;
}

static void print_error(CassFuture* future) {
  const char* message;
  size_t message_length;
  cass_future_error_message(future, &message, &message_length);
  ta_log_error("Error: %.*s\n", (int)message_length, message);
}

static CassCluster* create_cluster(const char* hosts) {
  CassCluster* cluster = cass_cluster_new();
  cass_cluster_set_contact_points(cluster, hosts);
  return cluster;
}

static CassError connect_session(CassSession* session, const CassCluster* cluster) {
  CassError rc = CASS_OK;
  CassFuture* future = cass_session_connect(session, cluster);

  cass_future_wait(future);
  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  }
  cass_future_free(future);

  return rc;
}

static CassError execute_query(CassSession* session, const char* query) {
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

static CassError prepare_query(CassSession* session, const char* query, const CassPrepared** prepared) {
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

static CassError execute_statement(CassSession* session, CassStatement* statement) {
  CassError rc = CASS_OK;
  CassFuture* future = NULL;
  future = cass_session_execute(session, statement);

  cass_future_wait(future);

  rc = cass_future_error_code(future);
  if (rc != CASS_OK) {
    print_error(future);
  }

  cass_future_free(future);
  cass_statement_free(statement);

  return rc;
}

static status_t get_blob(const CassRow* row, cass_byte_t* target, const char* column) {
  size_t len;
  const cass_byte_t* buf;
  status_t ret = SC_OK;
  cass_value_get_bytes(cass_row_get_column_by_name(row, column), &buf, &len);
  memcpy(target, buf, len);
  return ret;
}

static status_t make_query(char** result, const char* head_desc, const char* position, const char* left_desc) {
  if (head_desc == NULL || position == NULL || left_desc == NULL) {
    ta_log_error("NULL pointer to CQL query\n");
    return SC_TA_NULL;
  }
  size_t head_len = strlen(head_desc);
  size_t pos_len = strlen(position);
  size_t left_len = strlen(left_desc);
  size_t result_len = head_len + pos_len + left_len + 1;
  *result = malloc(result_len * sizeof(char));
  if (*result == NULL) {
    ta_log_error("%s\n", "SC_STORAGE_OOM");
    return SC_STORAGE_OOM;
  }
  memcpy(*result, head_desc, head_len);
  memcpy(*result + head_len, position, pos_len);
  memcpy(*result + head_len + pos_len, left_desc, left_len);
  (*result)[result_len - 1] = 0;

  return SC_OK;
}

static status_t create_keyspace(CassSession* session, const char* keyspace_name) {
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
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  free(create_query);
  return ret;
}

static status_t create_bundle_table(CassSession* session, bool need_drop) {
  status_t ret = SC_OK;
  if (need_drop) {
    if (execute_query(session, "DROP TABLE IF EXISTS bundleTable;") != CASS_OK) {
      ta_log_error("Drop bundleTable fail\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
  }
  if (execute_query(session,
                    "CREATE TABLE IF NOT EXISTS bundleTable ("
                    "bundle blob,"
                    "address blob,"
                    "hash blob,"
                    "message blob,"
                    "value bigint,"
                    "timestamp bigint,"
                    "trunk blob,"
                    "branch blob,"
                    "PRIMARY KEY (bundle, address, hash));") != CASS_OK) {
    ta_log_error("Create bundleTable fail\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }
exit:
  return ret;
}

static status_t create_edge_table(CassSession* session, bool need_drop) {
  status_t ret = SC_OK;
  if (need_drop) {
    if (execute_query(session, "DROP TABLE IF EXISTS edgeTable;") != CASS_OK) {
      ta_log_error("Drop edgeTable fail\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
  }
  if (execute_query(session,
                    "CREATE TABLE IF NOT EXISTS edgeTable("
                    "edge blob,"
                    "bundle blob,"
                    "address blob,"
                    "hash blob,"
                    "PRIMARY KEY (edge, bundle, address, hash));") != CASS_OK) {
    ta_log_error("Create edgeTable fail\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

exit:
  return ret;
}

static status_t create_identity_table(CassSession* session, bool need_drop) {
  status_t ret = SC_OK;
  if (need_drop) {
    if (execute_query(session, "DROP TABLE IF EXISTS identity;") != CASS_OK) {
      ta_log_error("drop identity table fail\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
  }
  if (execute_query(session,
                    "CREATE TABLE IF NOT EXISTS identity("
                    "id bigint, hash blob, status tinyint, PRIMARY KEY (id));") != CASS_OK) {
    ta_log_error("create identity table fail\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }
  if (execute_query(session, "CREATE INDEX IF NOT EXISTS ON identity(status);") != CASS_OK) {
    ta_log_error("create identity table index fail\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

exit:
  return ret;
}

status_t db_client_service_init(db_client_service_t* service, const char* host) {
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)\n");
    return SC_TA_NULL;
  }
  if (host == NULL) {
    ta_log_error("NULL pointer to ScyllaDB hostname\n");
    return SC_TA_NULL;
  }
  service->session = cass_session_new();
  service->cluster = create_cluster(host);
  if (connect_session(service->session, service->cluster) != CASS_OK) {
    ta_log_error("%s\n", "connect ScyllaDB cluster fail");
    return SC_STORAGE_CONNECT_FAIL;
  }
  return SC_OK;
}

status_t db_client_service_free(db_client_service_t* service) {
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)\n");
    return SC_TA_NULL;
  }
  if (service->session != NULL) {
    cass_session_free(service->session);
  }
  if (service->cluster != NULL) {
    cass_cluster_free(service->cluster);
  }
  return SC_OK;
}

status_t db_insert_tx_into_identity(db_client_service_t* service, int64_t id, const char* hash,
                                    db_txn_status_t status) {
  status_t ret = SC_OK;
  db_identity_t* identity = NULL;
  if ((ret = db_identity_new(&identity)) != SC_OK) {
    goto exit;
  }
  if ((ret = db_set_identity_hash(identity, (cass_byte_t*)hash, NUM_FLEX_TRITS_HASH)) != SC_OK) {
    goto exit;
  }
  if ((ret = db_set_identity_id(identity, id)) != SC_OK) {
    goto exit;
  }
  if ((ret = db_set_identity_status(identity, status)) != SC_OK) {
    goto exit;
  }
  if ((ret = db_insert_identity_table(service, identity)) != SC_OK) {
    goto exit;
  }

exit:
  db_identity_free(&identity);
  return ret;
}

status_t db_init_identity_keyspace(db_client_service_t* service, bool need_drop, const char* keyspace_name) {
  status_t ret = SC_OK;
  CassStatement* use_statement = NULL;
  char* use_query = NULL;
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)\n");
    return SC_TA_NULL;
  }
  if ((ret = create_keyspace(service->session, keyspace_name)) != SC_OK) {
    ta_log_error("create %s keyspace fail\n", keyspace_name);
    goto exit;
  }
  ret = make_query(&use_query, "USE ", keyspace_name, "");
  if (ret != SC_OK) {
    ta_log_error("%s\n", "make USE keyspace query fail");
    goto exit;
  }
  use_statement = cass_statement_new(use_query, 0);
  if (execute_statement(service->session, use_statement) != CASS_OK) {
    ta_log_error("USE keyspace %s fail\n", keyspace_name);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  if ((ret = create_identity_table(service->session, need_drop)) != SC_OK) {
    ta_log_error("%s\n", "create identity table fail");
    goto exit;
  }

exit:
  free(use_query);
  return ret;
}

status_t db_permanent_keyspace_init(db_client_service_t* service, bool need_drop, const char* keyspace_name) {
  status_t ret = SC_OK;
  CassStatement* use_statement = NULL;
  char* use_query = NULL;

  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)");
    return SC_TA_NULL;
  }
  if ((ret = create_keyspace(service->session, keyspace_name)) != SC_OK) {
    ta_log_error("%s\n", "create permanent keyspace fail");
    goto exit;
  }
  ret = make_query(&use_query, "USE ", keyspace_name, "");
  if (ret != SC_OK) {
    ta_log_error("%s\n", "make USE keyspace query fail");
    goto exit;
  }
  use_statement = cass_statement_new(use_query, 0);
  if (execute_statement(service->session, use_statement) != CASS_OK) {
    ta_log_error("Use keyspace : %s fail\n", keyspace_name);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  if ((ret = create_bundle_table(service->session, need_drop)) != SC_OK) {
    ta_log_error("%s\n", "create bundle table fail");
    goto exit;
  }
  if ((ret = create_edge_table(service->session, need_drop)) != SC_OK) {
    ta_log_error("%s\n", "create edge table fail");
    goto exit;
  }

exit:
  free(use_query);
  return ret;
}

static CassStatement* ret_insert_identity_statement(const CassPrepared* prepared, const db_identity_t* obj) {
  CassStatement* statement = NULL;
  statement = cass_prepared_bind(prepared);
  cass_statement_bind_int64_by_name(statement, "id", db_ret_identity_id(obj));
  cass_statement_bind_bytes_by_name(statement, "hash", db_ret_identity_hash(obj), NUM_FLEX_TRITS_HASH);
  cass_statement_bind_int8_by_name(statement, "status", db_ret_identity_status(obj));
  return statement;
}

status_t db_insert_identity_table(db_client_service_t* service, db_identity_t* obj) {
  status_t ret = SC_OK;
  const CassPrepared* insert_prepared = NULL;
  CassStatement* statement = NULL;
  const char* insert_query =
      "INSERT INTO identity (id, status, hash)"
      "VALUES (?, ?, ?);";
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)");
    return SC_TA_NULL;
  }
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }

  if (prepare_query(service->session, insert_query, &insert_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare INSERT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  statement = ret_insert_identity_statement(insert_prepared, obj);
  if (execute_statement(service->session, statement) != CASS_OK) {
    ta_log_error("insert obj into identity table fail\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

exit:
  cass_prepared_free(insert_prepared);
  return ret;
}

static status_t get_identity_array(CassSession* session, CassStatement* statement,
                                   db_identity_array_t* identity_array) {
  status_t ret = SC_OK;
  CassFuture* future = NULL;
  const CassResult* result;
  CassIterator* iterator;
  db_identity_t* identity = NULL;
  if ((ret = db_identity_new(&identity)) != SC_OK) {
    goto exit;
  }
  future = cass_session_execute(session, statement);
  cass_future_wait(future);
  if (cass_future_error_code(future) != CASS_OK) {
    print_error(future);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  result = cass_future_get_result(future);
  iterator = cass_iterator_from_result(result);

  while (cass_iterator_next(iterator)) {
    db_identity_t* itr;
    const cass_byte_t* hash;
    cass_int8_t value;
    cass_int64_t id;
    size_t len;
    const CassRow* row = cass_iterator_get_row(iterator);

    utarray_extend_back(identity_array);
    itr = (db_identity_t*)utarray_back(identity_array);

    cass_value_get_bytes(cass_row_get_column_by_name(row, "hash"), &hash, &len);
    db_set_identity_hash(itr, hash, NUM_FLEX_TRITS_HASH);
    cass_value_get_int8(cass_row_get_column_by_name(row, "status"), &value);
    db_set_identity_status(itr, value);
    cass_value_get_int64(cass_row_get_column_by_name(row, "id"), &id);
    db_set_identity_id(itr, id);
  }

  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);
  db_identity_free(&identity);
  return ret;
}

status_t db_select_identity_table(db_client_service_t* service, cass_int8_t status,
                                  db_identity_array_t* identity_array) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* select_prepared = NULL;
  const char* query = "SELECT * FROM identity WHERE status = ?;";

  if (prepare_query(service->session, query, &select_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare SELECT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  statement = cass_prepared_bind(select_prepared);
  cass_statement_bind_int8_by_name(statement, "status", status);
  get_identity_array(service->session, statement, identity_array);

  cass_prepared_free(select_prepared);
  return ret;
}

static CassStatement* ret_insert_bundle_statement(const CassPrepared* prepared,
                                                  const scylla_iota_transaction_t* transaction) {
  CassStatement* statement = NULL;
  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "bundle", transaction->bundle, NUM_FLEX_TRITS_BUNDLE);
  cass_statement_bind_bytes_by_name(statement, "address", transaction->address, NUM_FLEX_TRITS_ADDRESS);
  cass_statement_bind_bytes_by_name(statement, "hash", transaction->hash, NUM_FLEX_TRITS_HASH);
  cass_statement_bind_bytes_by_name(statement, "message", transaction->message, NUM_FLEX_TRITS_MESSAGE);
  cass_statement_bind_int64_by_name(statement, "value", transaction->value);
  cass_statement_bind_int64_by_name(statement, "timestamp", transaction->timestamp);
  cass_statement_bind_bytes_by_name(statement, "trunk", transaction->trunk, NUM_FLEX_TRITS_TRUNK);
  cass_statement_bind_bytes_by_name(statement, "branch", transaction->branch, NUM_FLEX_TRITS_BRANCH);
  return statement;
}

static CassStatement* ret_insert_edge_statement(const CassPrepared* prepared,
                                                const scylla_iota_transaction_t* transaction, const cass_byte_t* edge) {
  CassStatement* statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "edge", edge, NUM_FLEX_TRITS_HASH);
  cass_statement_bind_bytes_by_name(statement, "bundle", transaction->bundle, NUM_FLEX_TRITS_BUNDLE);
  cass_statement_bind_bytes_by_name(statement, "address", transaction->address, NUM_FLEX_TRITS_ADDRESS);
  cass_statement_bind_bytes_by_name(statement, "hash", transaction->hash, NUM_FLEX_TRITS_HASH);
  return statement;
}

static CassStatement* ret_select_from_bundleTable_statement(const CassPrepared* parpared, select_method_t select_method,
                                                            const select_where_t* select_where) {
  CassStatement* statement = cass_prepared_bind(parpared);
  if (select_method == WITH_BUNDLE) {
    cass_statement_bind_bytes_by_name(statement, "bundle", select_where->bundle, NUM_FLEX_TRITS_BUNDLE);
  } else if (select_method == WITH_BUNDLE_AND_ADDRESS) {
    cass_statement_bind_bytes_by_name(statement, "bundle", select_where->bundle, NUM_FLEX_TRITS_BUNDLE);
    cass_statement_bind_bytes_by_name(statement, "address", select_where->address, NUM_FLEX_TRITS_ADDRESS);
  }
  return statement;
}

status_t insert_transaction_into_bundleTable(db_client_service_t* service, scylla_iota_transaction_t* transaction,
                                             size_t trans_num) {
  status_t ret = SC_OK;
  const CassPrepared* insert_prepared = NULL;
  CassStatement* statement = NULL;
  const char* insert_query =
      "INSERT INTO bundleTable (bundle, address, hash, message, value, timestamp, trunk, branch)"
      "VALUES (?, ?, ?, ?, ?, ?, ?, ?);";
  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)");
    return SC_TA_NULL;
  }
  if (transaction == NULL) {
    ta_log_error("Invaild pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }

  if (prepare_query(service->session, insert_query, &insert_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare INSERT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  for (size_t i = 0; i < trans_num; i++) {
    statement = ret_insert_bundle_statement(insert_prepared, transaction + i);
    if (execute_statement(service->session, statement) != CASS_OK) {
      ta_log_error("Insert transactions into bundle table fail\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
  }

exit:
  cass_prepared_free(insert_prepared);
  return ret;
}

status_t insert_transaction_into_edgeTable(db_client_service_t* service, scylla_iota_transaction_t* transaction,
                                           size_t trans_num) {
  status_t ret = SC_OK;
  const CassPrepared* insert_prepared = NULL;
  CassStatement* statement = NULL;
  const char* insert_query =
      "INSERT INTO edgeTable (edge, bundle, address, hash)"
      "VALUES (?, ?, ?, ?);";

  if (transaction == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }

  if (prepare_query(service->session, insert_query, &insert_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare INSERT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  for (size_t i = 0; i < trans_num; i++) {
    statement = ret_insert_edge_statement(insert_prepared, transaction + i, (transaction + i)->address);
    if (execute_statement(service->session, statement) != CASS_OK) {
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
    statement = ret_insert_edge_statement(insert_prepared, transaction + i, (transaction + i)->trunk);
    if (execute_statement(service->session, statement) != CASS_OK) {
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
    statement = ret_insert_edge_statement(insert_prepared, transaction + i, (transaction + i)->branch);
    if (execute_statement(service->session, statement) != CASS_OK) {
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto exit;
    }
  }

exit:
  cass_prepared_free(insert_prepared);
  return ret;
}

static status_t select_from_bundleTable(CassSession* session, CassStatement* statement,
                                        scylla_iota_transaction_t** transaction, size_t* transactionNum) {
  int idx;
  status_t ret = SC_OK;
  CassFuture* future = NULL;
  const CassResult* result;
  CassIterator* iterator;

  future = cass_session_execute(session, statement);
  cass_future_wait(future);
  if (cass_future_error_code(future) != CASS_OK) {
    print_error(future);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  /* get transaction number */
  result = cass_future_get_result(future);
  iterator = cass_iterator_from_result(result);
  *transactionNum = 0;
  while (cass_iterator_next(iterator)) {
    (*transactionNum)++;
  }
  *transaction = malloc((*transactionNum) * sizeof(scylla_iota_transaction_t));
  if (*transaction == NULL) {
    ta_log_error("%s\n", "SC_STORAGE_OOM");
    ret = SC_STORAGE_OOM;
    goto end_iterate;
  }
  /* get each transaction result */
  idx = 0;
  iterator = cass_iterator_from_result(result);
  const CassRow* row;
  while (cass_iterator_next(iterator)) {
    row = cass_iterator_get_row(iterator);

    if ((ret = get_blob(row, ((*transaction) + idx)->bundle, "bundle")) != SC_OK) {
      goto end_iterate;
    }
    if ((ret = get_blob(row, ((*transaction) + idx)->address, "address")) != SC_OK) {
      goto end_iterate;
    }
    if ((ret = get_blob(row, ((*transaction) + idx)->hash, "hash")) != SC_OK) {
      goto end_iterate;
    }
    if ((ret = get_blob(row, ((*transaction) + idx)->message, "message")) != SC_OK) {
      goto end_iterate;
    }
    if ((ret = get_blob(row, ((*transaction) + idx)->trunk, "trunk")) != SC_OK) {
      goto end_iterate;
    }
    if ((ret = get_blob(row, ((*transaction) + idx)->branch, "branch")) != SC_OK) {
      goto end_iterate;
    }
    cass_value_get_int64(cass_row_get_column_by_name(row, "value"), &(((*transaction) + idx)->value));
    cass_value_get_int64(cass_row_get_column_by_name(row, "timestamp"), &(((*transaction) + idx)->timestamp));

    idx++;
  }

end_iterate:
  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);

  return ret;
}

static status_t push_columns_into_queue(CassSession* session, CassStatement* statement,
                                        hash243_queue_t* const res_queue, const char* column_name) {
  status_t ret = SC_OK;
  CassFuture* future = NULL;
  const CassResult* result;
  CassIterator* iterator;

  future = cass_session_execute(session, statement);
  cass_future_wait(future);

  if (cass_future_error_code(future) != CASS_OK) {
    print_error(future);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  result = cass_future_get_result(future);
  iterator = cass_iterator_from_result(result);

  while (cass_iterator_next(iterator)) {
    const CassRow* row = cass_iterator_get_row(iterator);
    size_t len;
    const cass_byte_t* buf;
    if (cass_value_get_bytes(cass_row_get_column_by_name(row, column_name), &buf, &len) != CASS_OK) {
      ta_log_error("get column : %s fail\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto end_iterate;
    }
    if (hash243_queue_push(res_queue, (flex_trit_t const* const)buf) != RC_OK) {
      ta_log_error("%s\n", "SC_STORAGE_OOM");
      ret = SC_STORAGE_OOM;
      goto end_iterate;
    }
  }

end_iterate:
  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);

  return ret;
}

status_t select_transactions_with_bundle(CassSession* session, const select_method_t select_method,
                                         const select_where_t* select_where, const size_t select_num) {
  status_t ret = SC_OK;
  scylla_iota_transaction_t* output;
  size_t outputNum = 0;
  CassStatement* statement = NULL;
  const CassPrepared* select_prepared = NULL;
  if (select_where == NULL) {
    ta_log_error("NULL pointer to select_where object\n");
    return SC_TA_NULL;
  }

  if (prepare_query(session, select_query[select_method].query, &select_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare SELECT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  for (size_t i = 0; i < select_num; i++) {
    statement = ret_select_from_bundleTable_statement(select_prepared, select_method, select_where + i);
    select_from_bundleTable(session, statement, &output, &outputNum);
    ta_log_debug("row = %d\n", outputNum);
    for (size_t i = 0; i < outputNum; i++) {
      ta_log_debug("%s %s %s %s %ld %ld %s %s\n", output[i].bundle, output[i].address, output[i].hash,
                   output[i].message, output[i].value, output[i].timestamp, output[i].trunk, output[i].branch);
    }
    free_scylla_iota_transaction(&output);
  }

  cass_prepared_free(select_prepared);
  return ret;
}

static status_t get_column_from_bundleTable(CassSession* session, hash243_queue_t* res_queue,
                                            const select_method_t select_method, const select_where_t* select_where,
                                            const char* column_name) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* select_prepared = NULL;
  if (select_where == NULL) {
    ta_log_error("NULL pointer to select_where object\n");
    return SC_TA_NULL;
  }
  if (prepare_query(session, select_query[select_method].query, &select_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare SELECT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  statement = ret_select_from_bundleTable_statement(select_prepared, select_method, select_where);
  ret = push_columns_into_queue(session, statement, res_queue, column_name);

  cass_prepared_free(select_prepared);
  return ret;
}
status_t get_column_from_edgeTable(db_client_service_t* service, hash243_queue_t* res_queue, cass_byte_t* edge,
                                   const char* column_name) {
  status_t ret = SC_OK;
  static const char* query = "SELECT * FROM edgeTable WHERE edge = ?";
  CassStatement* statement = NULL;
  const CassPrepared* select_prepared = NULL;

  if (prepare_query(service->session, query, &select_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare SELECT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  statement = cass_prepared_bind(select_prepared);
  cass_statement_bind_bytes_by_name(statement, "edge", edge, FLEX_TRIT_SIZE_243);
  ret = push_columns_into_queue(service->session, statement, res_queue, column_name);

  cass_prepared_free(select_prepared);
  return ret;
}

status_t get_transactions(db_client_service_t* service, hash243_queue_t* res_queue, hash243_queue_t bundles,
                          hash243_queue_t addresses, hash243_queue_t approves) {
  status_t ret = SC_OK;
  hash243_queue_t itr243 = NULL;
  hash243_queue_t itr243_2 = NULL;
  select_where_t select_where;

  CDL_FOREACH(bundles, itr243) {
    select_where.bundle = (cass_byte_t*)itr243->hash;
    ret = get_column_from_bundleTable(service->session, res_queue, WITH_BUNDLE, &select_where, "hash");
    if (ret != SC_OK) {
      goto exit;
    }
  }
  CDL_FOREACH(addresses, itr243) {
    hash243_queue_t bundle_hashes = NULL;
    ret = get_column_from_edgeTable(service, &bundle_hashes, (cass_byte_t*)itr243->hash, "bundle");
    if (ret != SC_OK) {
      hash243_queue_free(&bundle_hashes);
      goto exit;
    }
    select_where.address = (cass_byte_t*)itr243->hash;
    CDL_FOREACH(bundle_hashes, itr243_2) {
      select_where.bundle = (cass_byte_t*)itr243_2->hash;
      ret = get_column_from_bundleTable(service->session, res_queue, WITH_BUNDLE_AND_ADDRESS, &select_where, "hash");
      if (ret != SC_OK) {
        hash243_queue_free(&bundle_hashes);
        goto exit;
      }
    }
    hash243_queue_free(&bundle_hashes);
  }
  CDL_FOREACH(approves, itr243) {
    ret = get_column_from_edgeTable(service, res_queue, (cass_byte_t*)itr243->hash, "hash");
    if (ret != SC_OK) {
      goto exit;
    }
  }

exit:
  return ret;
}
