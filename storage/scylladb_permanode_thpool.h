/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef STORAGE_SCYLLADB_PERMANODE_THPOOL_H_
#define STORAGE_SCYLLADB_PERMANODE_THPOOL_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <pthread.h>
#include "scylladb_permanode.h"

/**
 * @file storage/scylladb_permanode_thpool.h
 * @brief Thread pool implematation for inserting data into permanode
 */

struct request {
  tryte_t hash[NUM_TRYTES_HASH]; /* number of the request                  */
  tryte_t trytes[NUM_TRYTES_SERIALIZED_TRANSACTION];
  struct request* next; /* pointer to next request, NULL if none. */
};

typedef struct {
  pthread_mutex_t request_mutex;
  pthread_cond_t got_request;
  pthread_cond_t finish_request;
  int num_requests; /* number of pending requests, initially none */
  int working_thread_num;
  struct request* requests;     /* head of linked list of requests. */
  struct request* last_request; /* pointer to last request.         */

} db_permanode_pool_t;

typedef struct {
  pthread_mutex_t thread_mutex;
  db_permanode_pool_t* pool;
  db_client_service_t service;
} db_worker_thread_t;

/**
 * @brief Initialize request, pthread mutex, pthread cond in threadpool struct
 *
 * @param[in] poll pointer to db_permanode_pool_t
 *
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_permanode_thpool_init(db_permanode_pool_t* pool);

/**
 * @brief Initialize work thread data
 *
 * @param[in] thread_data target worker thread struct
 * @param[in] pool connected thread pool
 * @param[in] host ScyllaDB host name
 *
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_permanode_thpool_init_worker(db_worker_thread_t* thread_data, db_permanode_pool_t* pool, char* host);

/**
 * @brief Add request into permanode request threadpoll
 *
 * @param[in] hash input transaction hash
 * @param[in] tryte input transaction trytes
 * @param[in] pool connected thread pool
 *
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_permanode_thpool_add(const tryte_t* hash, const tryte_t* trytes, db_permanode_pool_t* pool);

/**
 * @brief infinite loop of requests handling
 *
 * Loop forever, if there are requests to handle, take the first and handle it.
 * Then wait on the given condition variable, and when it is signaled, re-do the loop.
 *
 * @param[in] data pointer to db_permanode_pool_t
 *
 * @return NULL
 */
void* db_permanode_worker_handler(void* data);

/**
 * @brief Wait until the pool is empty and all workers are idle
 *
 * @param[in] data pointer to db_permanode_pool_t
 *
 * @return NULL
 */
void db_permanode_tpool_wait(db_permanode_pool_t* pool);

#ifdef __cplusplus
}
#endif

#endif  // STORAGE_SCYLLADB_PERMANODE_THPOOL_H_
