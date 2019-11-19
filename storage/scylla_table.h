/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef TA_SCYLLA_TABLE_H_
#define TA_SCYLLA_TABLE_H_
#ifdef __cplusplus
extern "C" {
#endif
#include "accelerator/errors.h"
#include "cassandra.h"
#include "utarray.h"

typedef struct db_identity_s db_identity_t;
typedef UT_array db_identity_array_t;

typedef enum { PENDING_TXN = 0, INSERTING_TXN, CONFIRMED_TXN, NUM_OF_TXN_STATUS } txn_status_t;

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
status_t db_new_identity(db_identity_t** obj);

/**
 * @brief release memory space to *obj
 *
 * @param[in] obj pointer to pointer to db_identity_t to be free
 */
void db_free_identity(db_identity_t** obj);

/**
 * @brief set uuid in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 * @param[in] uuid uuid to be set into db_identity_t
 * @return
 * - SC_OK on success
 * - SC_TA_NULL/SC_STORAGE_INVAILD_INPUT on error
 */
status_t db_set_identity_uuid(db_identity_t* obj, CassUuid uuid);

/**
 * @brief return uuid in db_identity_t
 *
 * @param[in] obj pointer to db_identity_t
 *
 * @return
 * - uuid on success
 * - NULL on error
 */
CassUuid db_ret_identity_uuid(const db_identity_t* obj);

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

#ifdef __cplusplus
}
#endif

#endif  // TA_SCYLLA_TABLE_H_