/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "scylladb_Permanode.h"
#include "common/trinary/trit_long.h"
#define logger_id scylladb_logger_id

#define ADDRESS_OFFSET NUM_TRYTES_SIGNATURE
#define VALUE_OFFSET (ADDRESS_OFFSET + NUM_TRYTES_ADDRESS)
#define OBSOLETE_TAG_OFFSET (VALUE_OFFSET + NUM_TRYTES_VALUE)
#define TIMESTAMP_OFFSET (OBSOLETE_TAG_OFFSET + NUM_TRYTES_OBSOLETE_TAG)
#define CURRENT_INDEX_OFFSET (TIMESTAMP_OFFSET + NUM_TRYTES_TIMESTAMP)
#define LAST_INDEX_OFFSET (CURRENT_INDEX_OFFSET + NUM_TRYTES_CURRENT_INDEX)
#define BUNDLE_OFFSET (LAST_INDEX_OFFSET + NUM_TRYTES_LAST_INDEX)
#define TRUNK_OFFSET (BUNDLE_OFFSET + NUM_TRYTES_BUNDLE)
#define BRNACH_OFFSET (TRUNK_OFFSET + NUM_TRYTES_TRUNK)

static void print_error(CassFuture* future) {
  const char* message;
  size_t message_length;
  cass_future_error_message(future, &message, &message_length);
  ta_log_error("Error: %.*s\n", (int)message_length, message);
}

static status_t create_transaction_table(CassSession* session, bool need_truncate) {
  if (execute_query(session,
                    "CREATE TABLE IF NOT EXISTS transaction ("
                    "hash blob PRIMARY KEY,"
                    "address blob,"
                    "bundle blob,"
                    "tag blob,"
                    "approvees set<blob>,"
                    "trytes blob );") != CASS_OK) {
    ta_log_error("Fail to create table : transaction\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  if (execute_query(session, "CREATE INDEX IF NOT EXISTS ON transaction(bundle);") != CASS_OK) {
    ta_log_error("Fail to create transaction table index : bundle\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  if (execute_query(session, "CREATE INDEX IF NOT EXISTS ON transaction(address);") != CASS_OK) {
    ta_log_error("Fail to create transaction table index : address\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  if (execute_query(session, "CREATE INDEX IF NOT EXISTS ON transaction(tag);") != CASS_OK) {
    ta_log_error("Fail to create transaction table index : tag\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  if (need_truncate) {
    if (db_truncate_table(session, "transaction") != SC_OK) {
      ta_log_error("Fail to truncate table : transaction\n");
      return SC_STORAGE_CASSANDRA_QUREY_FAIL;
    }
  }
  return SC_OK;
}

static CassStatement* ret_insert_transaction_statement(const CassPrepared* prepared, const tryte_t* hash,
                                                       const tryte_t* trytes) {
  CassStatement* statement = NULL;
  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "hash", (cass_byte_t*)hash, NUM_TRYTES_HASH);
  cass_statement_bind_bytes_by_name(statement, "bundle", (cass_byte_t*)(trytes + BUNDLE_OFFSET), NUM_TRYTES_BUNDLE);
  cass_statement_bind_bytes_by_name(statement, "address", (cass_byte_t*)(trytes + ADDRESS_OFFSET), NUM_TRYTES_ADDRESS);
  cass_statement_bind_bytes_by_name(statement, "tag", (cass_byte_t*)(trytes + OBSOLETE_TAG_OFFSET),
                                    NUM_TRYTES_OBSOLETE_TAG);
  cass_statement_bind_bytes_by_name(statement, "trytes", (cass_byte_t*)trytes, NUM_TRYTES_SERIALIZED_TRANSACTION);
  return statement;
}

static status_t insert_transaction(CassSession* session, const tryte_t* hash, const tryte_t* trytes) {
  status_t ret = SC_OK;
  const CassPrepared* prepared = NULL;
  CassStatement* statement = NULL;
  const char* query = "UPDATE transaction Set bundle = ?, address = ?, tag = ?, trytes = ?  Where hash = ?";

  if (session == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client session for connection endpoint(s)");
    return SC_TA_NULL;
  }
  if (hash == NULL || trytes == NULL) {
    ta_log_error("NULL trytes pointer\n");
    return SC_TA_NULL;
  }

  if (prepare_query(session, query, &prepared) != CASS_OK) {
    ta_log_error("Fail to prepare query : %s\n", query);
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  statement = ret_insert_transaction_statement(prepared, hash, trytes);
  if (execute_statement(session, statement) != CASS_OK) {
    ta_log_error("Fail to insert transactions into transaction table\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  cass_prepared_free(prepared);
  return ret;
}

static status_t insert_approvee(CassSession* session, const tryte_t* hash, const tryte_t* approvee) {
  status_t ret = SC_OK;
  const CassPrepared* prepared = NULL;
  CassStatement* statement = NULL;
  CassCollection* collection = NULL;

  if (hash == NULL || approvee == NULL) {
    ta_log_error("NULL pointer to ScyllaDB transaction object\n");
    return SC_TA_NULL;
  }

  char* query = "UPDATE transaction SET approvees = approvees + ? WHERE hash = ?";
  if (prepare_query(session, query, &prepared) != CASS_OK) {
    ta_log_error("Fail to prepare query : %s\n", query);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }
  statement = cass_prepared_bind(prepared);
  collection = cass_collection_new(CASS_COLLECTION_TYPE_SET, 1);
  cass_collection_append_bytes(collection, (cass_byte_t*)approvee, NUM_TRYTES_HASH);
  cass_statement_bind_collection(statement, 0, collection);
  cass_collection_free(collection);
  cass_statement_bind_bytes(statement, 1, (cass_byte_t*)hash, NUM_TRYTES_HASH);
  if (execute_statement(session, statement) != CASS_OK) {
    ta_log_error("Fail to insert approvee\n");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  cass_prepared_free(prepared);
exit:

  return ret;
}

static status_t push_collection_into_queue(CassSession* session, CassStatement* statement,
                                           hash243_queue_t* const res_queue, const char* column_name) {
  status_t ret = SC_CCLIENT_NOT_FOUND;
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
  if (cass_iterator_next(iterator)) {
    const CassValue* value = NULL;
    const CassRow* row = cass_iterator_get_row(iterator);
    CassIterator* items_iterator = NULL;
    value = cass_row_get_column_by_name(row, column_name);

    items_iterator = cass_iterator_from_collection(value);
    while (cass_iterator_next(items_iterator)) {
      size_t len;
      const cass_byte_t* buf;
      cass_value_get_bytes(cass_iterator_get_value(items_iterator), &buf, &len);
      if (hash243_queue_push(res_queue, (flex_trit_t*)buf) != RC_OK) {
        ta_log_error("%s\n", "SC_STORAGE_OOM");
        ret = SC_STORAGE_OOM;
        goto end_iterate;
      }
      ret = SC_OK;
    }

  end_iterate:
    cass_iterator_free(items_iterator);
  }

  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);

  return ret;
}

static status_t get_count(CassSession* session, CassStatement* statement, get_inclusion_states_res_t* const res) {
  status_t ret = SC_CCLIENT_NOT_FOUND;
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

  if (cass_iterator_next(iterator)) {
    const CassRow* row = cass_iterator_get_row(iterator);
    cass_int64_t count;

    if (cass_value_get_int64(cass_row_get_column(row, 0), &count) != CASS_OK) {
      ta_log_error("Get column failed\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto end_iterate;
    }
    if (get_inclusion_states_res_states_add(res, (count > 0)) != RC_OK) {
      ta_log_error("Fail to push buf into queue\n");
      ret = SC_STORAGE_OOM;
      goto end_iterate;
    }
    ret = SC_OK;
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
  status_t ret = SC_CCLIENT_NOT_FOUND;
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
      ta_log_error("Get column failed\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto end_iterate;
    }
    if (hash243_queue_push(res_queue, (flex_trit_t*)buf) != RC_OK) {
      ta_log_error("Fail to push buf into queue\n");
      ret = SC_STORAGE_OOM;
      goto end_iterate;
    }
    ret = SC_OK;
  }

end_iterate:
  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);

  return ret;
}

static status_t get_trytes(CassSession* session, CassStatement* statement, hash8019_queue_t* res_queue) {
  status_t ret = SC_CCLIENT_NOT_FOUND;
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

  if (cass_iterator_next(iterator)) {
    const CassRow* row = cass_iterator_get_row(iterator);
    size_t len;
    const cass_byte_t* buf;
    if (cass_value_get_bytes(cass_row_get_column_by_name(row, "trytes"), &buf, &len) != CASS_OK) {
      ta_log_error("Fail to get column : trytes\n");
      ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
      goto end_iterate;
    }
    if (hash8019_queue_push(res_queue, (flex_trit_t const* const)buf) != RC_OK) {
      ta_log_error("%s\n", "SC_STORAGE_OOM");
      ret = SC_STORAGE_OOM;
    }
    ret = SC_OK;
  }

end_iterate:
  cass_result_free(result);
  cass_iterator_free(iterator);

exit:
  cass_future_free(future);
  cass_statement_free(statement);
  return ret;
}

static status_t get_inclustion_status_from_transaction_table(CassSession* session, get_inclusion_states_res_t* res,
                                                             const flex_trit_t* hash) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* prepared = NULL;

  char* query = "SELECT COUNT(*) FROM transaction WHERE hash = ? LIMIT 1";
  if (prepare_query(session, query, &prepared) != CASS_OK) {
    ta_log_error("%s\n", "Prepare SELECT query failed");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "hash", (cass_byte_t*)hash, NUM_TRYTES_HASH);
  ret = get_count(session, statement, res);
  cass_prepared_free(prepared);
  return ret;
}

static status_t get_approvee_from_transaction_table(CassSession* session, hash243_queue_t* res_queue,
                                                    const flex_trit_t* hash) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* prepared = NULL;

  char* query = "SELECT approvees FROM transaction WHERE hash = ?";
  if (prepare_query(session, query, &prepared) != CASS_OK) {
    ta_log_error("%s\n", "Prepare SELECT query failed");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "hash", (cass_byte_t*)hash, NUM_TRYTES_HASH);
  ret = push_collection_into_queue(session, statement, res_queue, "approvees");
  cass_prepared_free(prepared);
  return ret;
}

static status_t get_hash_from_transaction_table(CassSession* session, hash243_queue_t* res_queue,
                                                const char* where_column, const flex_trit_t* where_value,
                                                size_t length) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* prepared = NULL;
  char* query = NULL;
  make_query(&query, "SELECT hash FROM transaction WHERE ", where_column, " = ? LIMIT 100");
  if (prepare_query(session, query, &prepared) != CASS_OK) {
    ta_log_error("%s\n", "Prepare SELECT query failed");
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, where_column, (cass_byte_t*)where_value, length);
  ret = push_columns_into_queue(session, statement, res_queue, "hash");
  cass_prepared_free(prepared);

exit:
  free(query);
  return ret;
}

static status_t get_trytes_from_transaction_table(CassSession* session, hash8019_queue_t* res_trytes,
                                                  const flex_trit_t* hash) {
  status_t ret = SC_OK;
  CassStatement* statement = NULL;
  const CassPrepared* prepared = NULL;

  char* query = "SELECT trytes FROM transaction WHERE hash = ?";
  if (prepare_query(session, query, &prepared) != CASS_OK) {
    ta_log_error("%s\n", "Prepare SELECT query failed");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  statement = cass_prepared_bind(prepared);
  cass_statement_bind_bytes_by_name(statement, "hash", (cass_byte_t*)hash, NUM_TRYTES_HASH);
  ret = get_trytes(session, statement, res_trytes);
  cass_prepared_free(prepared);
  return ret;
}

status_t db_permanode_insert_transaction(const db_client_service_t* service, const tryte_t* hash,
                                         const tryte_t* trytes) {
  status_t ret = SC_OK;

  ret = insert_transaction(service->session, hash, trytes);
  if (ret != SC_OK) {
    ta_log_error("Fail to insert transaction\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  ret = insert_approvee(service->session, trytes + TRUNK_OFFSET, hash);
  if (ret != SC_OK) {
    ta_log_error("Fail to insert approvee\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }
  ret = insert_approvee(service->session, trytes + BRNACH_OFFSET, hash);
  if (ret != SC_OK) {
    ta_log_error("Fail to insert approvee\n");
    return SC_STORAGE_CASSANDRA_QUREY_FAIL;
  }

  return ret;
}

status_t db_get_transactions_by_bundle(const db_client_service_t* service, hash243_queue_t* res,
                                       const flex_trit_t* bundle) {
  if (service == NULL || res == NULL || bundle == NULL) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  return get_hash_from_transaction_table(service->session, res, "bundle", bundle, NUM_TRYTES_BUNDLE);
}

status_t db_get_transactions_by_address(const db_client_service_t* service, hash243_queue_t* res,
                                        const flex_trit_t* address) {
  if (service == NULL || res == NULL || address == NULL) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  return get_hash_from_transaction_table(service->session, res, "address", address, NUM_TRYTES_ADDRESS);
}

status_t db_get_transactions_by_tag(const db_client_service_t* service, hash243_queue_t* res, const flex_trit_t* tag) {
  if (service == NULL || res == NULL || tag == NULL) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  return get_hash_from_transaction_table(service->session, res, "tag", tag, NUM_TRYTES_OBSOLETE_TAG);
}

status_t db_get_approvee(const db_client_service_t* service, hash243_queue_t* res, const flex_trit_t* hash) {
  if (service == NULL || res == NULL || hash == NULL) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  return get_approvee_from_transaction_table(service->session, res, hash);
}

status_t db_get_trytes(const db_client_service_t* service, hash8019_queue_t* res, const flex_trit_t* hash) {
  if (service == NULL || res == NULL || hash == NULL) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  return get_trytes_from_transaction_table(service->session, res, hash);
}

status_t db_client_find_transactions(const db_client_service_t* service, const find_transactions_req_t* req,
                                     find_transactions_res_t* const res) {
  status_t ret = SC_OK;
  hash243_queue_t itr243 = NULL;
  hash81_queue_t itr81 = NULL;

  CDL_FOREACH(req->bundles, itr243) {
    ret = db_get_transactions_by_bundle(service, &res->hashes, itr243->hash);
    if (ret != SC_OK) {
      return ret;
    }
  }
  CDL_FOREACH(req->addresses, itr243) {
    ret = db_get_transactions_by_address(service, &res->hashes, itr243->hash);
    if (ret != SC_OK) {
      return ret;
    }
  }
  CDL_FOREACH(req->tags, itr81) {
    ret = db_get_transactions_by_tag(service, &res->hashes, itr81->hash);
    if (ret != SC_OK) {
      return ret;
    }
  }
  CDL_FOREACH(req->approvees, itr243) {
    ret = db_get_approvee(service, &res->hashes, itr243->hash);
    if (ret != SC_OK) {
      return ret;
    }
  }
  return ret;
}

status_t db_client_get_trytes(const db_client_service_t* service, get_trytes_req_t const* const req,
                              get_trytes_res_t* res) {
  status_t ret = SC_OK;
  hash243_queue_t itr243 = NULL;
  if (!service || !req || !res) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  CDL_FOREACH(req->hashes, itr243) {
    ret = db_get_trytes(service, &res->trytes, itr243->hash);
    if (ret != SC_OK) {
      return ret;
    }
  }
  return ret;
}

status_t db_client_get_inclusion_states(const db_client_service_t* service, get_inclusion_states_req_t const* const req,
                                        get_inclusion_states_res_t* res) {
  status_t ret = SC_OK;
  hash243_queue_t itr243 = NULL;

  if (!service || !req || !res) {
    ta_log_error("Invalid NULL pointer\n");
    return SC_TA_NULL;
  }
  CDL_FOREACH(req->transactions, itr243) {
    ret = get_inclustion_status_from_transaction_table(service->session, res, itr243->hash);
    if (ret != SC_OK) {
      ta_log_error("Fail to get inclustion status\n");
      return ret;
    }
    ta_log_debug("Inclusion status : %d, hash %s\n",
                 get_inclusion_states_res_states_at(res, get_inclusion_states_res_states_count(res) - 1), itr243->hash);
  }

  return ret;
}

status_t db_client_get_transaction_objects(const db_client_service_t* service, const get_trytes_req_t* tx_hashes,
                                           transaction_array_t* out_tx_objs) {
  status_t ret = SC_OK;
  hash8019_queue_entry_t* q_iter = NULL;
  iota_transaction_t tx;
  size_t tx_deserialize_offset = 0;
  get_trytes_res_t* out_trytes = get_trytes_res_new();

  if (!out_trytes) {
    ret = SC_TA_OOM;
    ta_log_error("Create get trytes response failed\n");
    goto done;
  }

  ret = db_client_get_trytes(service, tx_hashes, out_trytes);
  if (ret == SC_OK) {
    CDL_FOREACH(out_trytes->trytes, q_iter) {
      tx_deserialize_offset = transaction_deserialize_from_trits(&tx, q_iter->hash, true);
      if (tx_deserialize_offset) {
        transaction_array_push_back(out_tx_objs, &tx);
      } else {
        ta_log_error("Failed to deserialize transactions from tirts\n");
        goto done;
      }
    }
  }

done:
  get_trytes_res_free(&out_trytes);
  return ret;
}

status_t db_permanode_keyspace_init(const db_client_service_t* service, bool need_truncate, const char* keyspace_name) {
  status_t ret = SC_OK;
  CassStatement* use_statement = NULL;
  char* use_query = NULL;

  if (service == NULL) {
    ta_log_error("NULL pointer to ScyllaDB client service for connection endpoint(s)\n");
    return SC_TA_NULL;
  }
  if ((ret = create_keyspace(service->session, keyspace_name)) != SC_OK) {
    ta_log_error("%s\n", "Create permanent keyspace failed");
    goto exit;
  }
  ret = make_query(&use_query, "USE ", keyspace_name, "");
  if (ret != SC_OK) {
    ta_log_error("%s\n", "Make USE keyspace query failed");
    goto exit;
  }

  use_statement = cass_statement_new(use_query, 0);
  if (execute_statement(service->session, use_statement) != CASS_OK) {
    ta_log_error("Use keyspace : %s fail\n", keyspace_name);
    ret = SC_STORAGE_CASSANDRA_QUREY_FAIL;
    goto exit;
  }

  if ((ret = create_transaction_table(service->session, need_truncate)) != SC_OK) {
    ta_log_error("%s\n", "Create transaction table failed");
    goto exit;
  }

exit:
  free(use_query);
  return ret;
}