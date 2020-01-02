/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "scylladb_identity.h"
#include "common/model/transaction.h"
#include "time.h"

#define logger_id scylladb_logger_id

#define DB_IDENTITY_UUID_VERSION 4

struct db_identity_s {
  CassUuid uuid;
  cass_int64_t timestamp;
  cass_int8_t status;
  cass_byte_t hash[NUM_FLEX_TRITS_BUNDLE];
};

status_t db_identity_new(db_identity_t** obj) {
  *obj = (db_identity_t*)malloc(sizeof(struct db_identity_s));
  if (NULL == *obj) {
    ta_log_error("SC_STORAGE_OOM\n");
    return SC_STORAGE_OOM;
  }

  return SC_OK;
}

void db_identity_free(db_identity_t** obj) {
  free(*obj);
  *obj = NULL;
}

status_t db_set_identity_uuid(db_identity_t* obj, CassUuid* in) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  if (in == NULL) {
    ta_log_error("NULL pointer to uuid\n");
    return SC_TA_NULL;
  }
  /**< The version number is in the most significant 4 bits of the timestamp */
  int version = (in->time_and_version) >> 60;
  if (version != DB_IDENTITY_UUID_VERSION) {
    ta_log_error("input uuid version %d does not match expected version %d\n", version, DB_IDENTITY_UUID_VERSION);
    return SC_STORAGE_INVAILD_INPUT;
  }
  obj->uuid.time_and_version = in->time_and_version;
  obj->uuid.clock_seq_and_node = in->clock_seq_and_node;
  return SC_OK;
}

status_t db_get_identity_uuid_string(const db_identity_t* obj, char* res_uuid_string) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  if (res_uuid_string == NULL) {
    ta_log_error("NULL pointer to uuid string\n");
    return SC_TA_NULL;
  }
  cass_uuid_string(obj->uuid, res_uuid_string);
  return SC_OK;
}

status_t db_set_identity_status(db_identity_t* obj, cass_int8_t status) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  if (status < 0 || status >= NUM_OF_TXN_STATUS) {
    ta_log_error("invalid status : %d\n", status);
    return SC_STORAGE_INVAILD_INPUT;
  }
  obj->status = status;
  return SC_OK;
}

cass_int8_t db_ret_identity_status(const db_identity_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return 0;
  }
  return obj->status;
}

status_t db_set_identity_timestamp(db_identity_t* obj, cass_int64_t ts) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  obj->timestamp = ts;
  return SC_OK;
}

cass_int64_t db_ret_identity_timestamp(const db_identity_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return 0;
  }
  return obj->timestamp;
}

cass_int64_t db_ret_identity_time_elapsed(db_identity_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  return (cass_int64_t)time(NULL) - obj->timestamp;
}

status_t db_set_identity_hash(db_identity_t* obj, const cass_byte_t* hash, size_t length) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  if (hash == NULL) {
    ta_log_error("NULL pointer to hash to insert into identity table\n");
  }
  if (length != NUM_FLEX_TRITS_HASH) {
    ta_log_error("SC_STORAGE_INVAILD_INPUT\n");
    return SC_STORAGE_INVAILD_INPUT;
  }
  memcpy(obj->hash, hash, NUM_FLEX_TRITS_HASH);
  return SC_OK;
}

const cass_byte_t* db_ret_identity_hash(const db_identity_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return NULL;
  }
  return obj->hash;
}

static UT_icd db_identity_array_icd = {sizeof(db_identity_t), 0, 0, 0};

db_identity_array_t* db_identity_array_new() {
  db_identity_array_t* obj;
  utarray_new(obj, &db_identity_array_icd);
  return obj;
}

static void print_error(CassFuture* future) {
  const char* message;
  size_t message_length;
  cass_future_error_message(future, &message, &message_length);
  ta_log_error("Error: %.*s\n", (int)message_length, message);
}

static status_t create_identity_table(CassSession* session, bool need_drop) {
  if (need_drop) {
    if (execute_query(session, "DROP TABLE IF EXISTS identity;") != CASS_OK) {
      ta_log_error("drop identity table fail\n");
      return SC_STORAGE_CASSANDRA_QUREY_FAIL;
    }
  }
  if (execute_query(session,
                    "CREATE TABLE IF NOT EXISTS identity("
                    "id uuid, ts timestamp, hash blob, status tinyint, PRIMARY KEY (id));") != CASS_OK) {
    ta_log_error("create identity table fail\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  if (execute_query(session, "CREATE INDEX IF NOT EXISTS ON identity(status);") != CASS_OK) {
    ta_log_error("create identity table index fail\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  if (execute_query(session, "CREATE INDEX IF NOT EXISTS ON identity(hash);") != CASS_OK) {
    ta_log_error("create identity table index fail\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  return SC_OK;
}

status_t db_insert_tx_into_identity(db_client_service_t* service, const char* hash, db_txn_status_t status,
                                    char* res_uuid_string) {
  status_t ret = SC_OK;
  db_identity_t* identity = NULL;

  if ((ret = db_identity_new(&identity)) != SC_OK) {
    ta_log_error("db new identity failed\n");
    goto exit;
  }
  if ((ret = db_set_identity_hash(identity, (cass_byte_t*)hash, NUM_FLEX_TRITS_HASH)) != SC_OK) {
    ta_log_error("db set identity hash failed\n");
    goto exit;
  }
  if ((ret = db_set_identity_status(identity, status)) != SC_OK) {
    ta_log_error("db set identity status failed\n");
    goto exit;
  }
  if ((ret = db_set_identity_timestamp(identity, time(NULL))) != SC_OK) {
    ta_log_error("db set identity timestamp failed\n");
    goto exit;
  }

  cass_uuid_gen_random(service->uuid_gen, &identity->uuid);
  if (res_uuid_string != NULL) {
    cass_uuid_string(identity->uuid, res_uuid_string);
  }
  if ((ret = db_insert_identity_table(service, identity)) != SC_OK) {
    ta_log_error("db insert identity table failed\n");
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

static CassStatement* ret_insert_identity_statement(const CassPrepared* prepared, const db_identity_t* obj) {
  CassStatement* statement = NULL;
  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "hash", obj->hash, NUM_FLEX_TRITS_HASH);
  cass_statement_bind_int8_by_name(statement, "status", obj->status);
  cass_statement_bind_int64_by_name(statement, "ts", obj->timestamp);
  cass_statement_bind_uuid_by_name(statement, "id", obj->uuid);
  return statement;
}

status_t db_insert_identity_table(db_client_service_t* service, db_identity_t* obj) {
  status_t ret = SC_OK;
  const CassPrepared* insert_prepared = NULL;
  CassStatement* statement = NULL;
  const char* insert_query =
      "INSERT INTO identity (id, ts, status, hash)"
      "VALUES (?, ?, ?, ?);";
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
    CassUuid uuid;
    cass_int64_t ts;
    size_t len;
    const CassRow* row = cass_iterator_get_row(iterator);

    utarray_extend_back(identity_array);
    itr = (db_identity_t*)utarray_back(identity_array);

    cass_value_get_bytes(cass_row_get_column_by_name(row, "hash"), &hash, &len);
    db_set_identity_hash(itr, hash, NUM_FLEX_TRITS_HASH);
    cass_value_get_int8(cass_row_get_column_by_name(row, "status"), &value);
    db_set_identity_status(itr, value);
    cass_value_get_int64(cass_row_get_column_by_name(row, "ts"), &ts);
    db_set_identity_timestamp(itr, ts);
    cass_value_get_uuid(cass_row_get_column_by_name(row, "id"), &uuid);
    db_set_identity_uuid(itr, &uuid);
  }

  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);
  db_identity_free(&identity);
  return ret;
}

status_t db_get_identity_objs_by_status(db_client_service_t* service, cass_int8_t status,
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

status_t db_get_identity_objs_by_uuid_string(db_client_service_t* service, const char* uuid_string,
                                             db_identity_array_t* identity_array) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* select_prepared = NULL;
  const char* query = "SELECT * FROM identity WHERE id = ?;";
  CassUuid uuid;
  cass_uuid_from_string(uuid_string, &uuid);
  if (prepare_query(service->session, query, &select_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare SELECT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  statement = cass_prepared_bind(select_prepared);
  cass_statement_bind_uuid_by_name(statement, "id", uuid);
  get_identity_array(service->session, statement, identity_array);

  cass_prepared_free(select_prepared);
  return ret;
}

status_t db_get_identity_objs_by_hash(db_client_service_t* service, const cass_byte_t* hash,
                                      db_identity_array_t* identity_array) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* select_prepared = NULL;
  const char* query = "SELECT * FROM identity WHERE hash = ?;";

  if (prepare_query(service->session, query, &select_prepared) != CASS_OK) {
    ta_log_error("%s\n", "prepare SELECT query fail");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  statement = cass_prepared_bind(select_prepared);
  cass_statement_bind_bytes_by_name(statement, "hash", hash, NUM_FLEX_TRITS_HASH);
  get_identity_array(service->session, statement, identity_array);

  cass_prepared_free(select_prepared);
  return ret;
}
