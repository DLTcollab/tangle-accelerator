/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include "scylla_table.h"
#include "common/model/transaction.h"
#include "utils/logger.h"

static logger_id_t logger_id;

void scylla_table_logger_init() { logger_id = logger_helper_enable("scylla_table", LOGGER_DEBUG, true); }

int scylla_table_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(logger_id, "%s.\n", "scylla_table");
    return EXIT_FAILURE;
  }
  return 0;
}

struct db_identity_s {
  cass_int64_t id;
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

status_t db_set_identity_id(db_identity_t* obj, cass_int64_t id) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return SC_TA_NULL;
  }
  if (id < 0) {
    ta_log_error("Invaild input : nagative ID\n");
    return SC_STORAGE_INVAILD_INPUT;
  }
  obj->id = id;
  return SC_OK;
}

cass_int64_t db_ret_identity_id(const db_identity_t* obj) {
  if (obj == NULL) {
    ta_log_error("NULL pointer to ScyllaDB identity object\n");
    return INT64_MAX;
  }
  return obj->id;
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
