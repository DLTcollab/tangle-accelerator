/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_CACHE_H_
#define UTILS_CACHE_H_

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common/ta_errors.h"
#include "common/trinary/flex_trit.h"
#include "utils/handles/lock.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils/cache/cache.h
 * @brief Caching service interface
 * @example test_cache.c
 */

/** struct of cache_t */
typedef struct {
  /** @cond private data */
  void* conn;
  /** @endcond */
} cache_t;

/**
 * Initialize logger
 */
void br_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int br_logger_release();

/**
 * @brief Initiate cache module. This function can be called in 'config.c' only.
 *
 * @param rwlock[in,out] Read/Write lock object.
 * @param input_state[in] Whether cache server should be activated
 * @param host[in] cache server host
 * @param port[in] cache server port
 * @return
 * - True on success
 * - False on error
 */
bool cache_init(pthread_rwlock_t** rwlock, bool input_state, const char* host, int port);

/**
 * @brief Stop interacting with cache module. This function can be called in 'config.c' only.
 *
 * @param rwlock[in,out] Read/Write lock object.
 */
void cache_stop(pthread_rwlock_t** rwlock);

/**
 * @brief Delete certain key-value storage from cache
 *
 * @param key[in] Key string to search
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_del(const char* const key);

/**
 * @brief Get key-value storage from in-memory cache
 *
 * @param key[in] Key string to search
 * @param res[out] Result of GET key
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_get(const char* const key, char* res);

/**
 * @brief Set key-value storage in in-memory cache
 *
 * @param key[in] Key string to store
 * @param key_size[in] Size of key string to store
 * @param value[in] Value string to store
 * @param value_size[in] Size of value string to store
 * @param timeout[in] Set the timeout second of the key. If arg timeout equal less than 0, then no timeout will be set.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_set(const char* const key, const int key_size, const void* const value, const int value_size,
                   const int timeout);

/**
 * @brief Push an element to a list in in-memory cache
 *
 * @param key[in] Key string to store
 * @param key_size[in] Size of key string to store
 * @param value[in] Value string to store
 * @param value_size[in] Size of value string to store
 * @param timeout[in] Set the timeout of the key in second. If arg timeout is equal less than 0, then no timeout will be
 * set.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_push(const char* const key, const int key_size, const void* const value, const int value_size);

/**
 * @brief Get an element of a list from in-memory cache
 *
 * @param key[in] Key string to search
 * @param index[in] Assigned index for fetching element
 * @param res_len[in] Expected length of the response
 * @param res[out] The element with assigned index
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_at(const char* const key, const int index, const int res_len, char* res);

/**
 * @brief Get the top element of a list from in-memory cache
 *
 * @param key[in] Key string to search
 * @param res_len[in] Expected length of the response
 * @param res[out] The element with assigned index
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_peek(const char* const key, const int res_len, char* res);

/**
 * @brief Fetch the length of the list in in-memory cache
 *
 * @param key[in] Key string to store
 * @param len[out] The returned length of list
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_size(const char* const key, int* len);

/**
 * @brief Push an element to a list in in-memory cache
 *
 * @param key[in] Key string to store
 * @param key_size[in] Size of key string to store
 * @param value[in] Value string to store
 * @param value_size[in] Size of value string to store
 * @param exist[out] Whether the given value existed in the key storage.
 * set.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_exist(const char* const key, const char* const value, const int value_len, bool* exist);

/**
 * @brief Pop an element from the list in in-memory cache
 *
 * @param key[in] Key for key-value storage
 * @param res[out] The element with assigned index
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_pop(const char* const key, char* res);

/**
 * @brief Get the occupied size by Redis in bytes.
 *
 * @return
 * - Used size of the cache database on success
 * - -1 on error
 */
long int cache_occupied_space();

#ifdef __cplusplus
}
#endif

#endif  // UTILS_CACHE_H_
