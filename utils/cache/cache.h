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
 * Initiate cache module
 *
 * @param[in] state if cache server should open
 * @param[in] host cache server host
 * @param[in] port cache server port
 * @return
 * - True on success
 * - False on error
 */
bool cache_init(bool state, const char* host, int port);

/**
 * Stop interacting with cache module
 */
void cache_stop();

/**
 * Delete certain key-value store from cache
 *
 * @param[in] key Key string to search
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_del(const char* const key);

/**
 * Get key-value store from in-memory cache
 *
 * @param[in] key Key string to search
 * @param[out] res Result of GET key
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_get(const char* const key, char* res);

/**
 * Set key-value store into in-memory cache
 *
 * @param[in] cache Data type for Cache module
 * @param[in] key Key string to store
 * @param[in] key_size Size of key string to store
 * @param[in] value Value string to store
 * @param[in] value_size Size of value string to store
 * @param[in] timeout Set the timeout second of the key. If arg timeout equal less than 0, then no timeout will be set.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_set(const char* const key, const int key_size, const void* const value, const int value_size,
                   const int timeout);

/**
 * Push an elements to a list into in-memory cache
 *
 * @param[in] key Key string to store
 * @param[in] key_size Size of key string to store
 * @param[in] value Value string to store
 * @param[in] value_size Size of value string to store
 * @param[in] timeout Set the timeout of the key in second. If arg timeout is equal less than 0, then no timeout will be
 * set.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_push(const char* const key, const int key_size, const void* const value, const int value_size);

/**
 * Get an element of a list from in-memory cache
 *
 * @param[in] key Key string to search
 * @param[in] index Assigned index for fetching element
 * @param[in] res_len Expected length of the respose
 * @param[out] res The element with assigning index
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_at(const char* const key, const int index, const int res_len, char* res);

/**
 * Get the top element of a list from in-memory cache
 *
 * @param[in] key Key string to search
 * @param[in] res_len Expected length of the respose
 * @param[out] res The element with assigning index
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_peek(const char* const key, const int res_len, char* res);

/**
 * Fetch the length of the list in in-memory cache
 *
 * @param[in] key Key string to store
 * @param[out] len The returned length of list
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_size(const char* const key, int* len);

/**
 * Push an elements to a list into in-memory cache
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
 * Pop an element from the list in in-memory cache
 *
 * @param[in] key Key for key-value starage
 * @param[out] res The element with assigning index
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_list_pop(const char* const key, char* res);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_CACHE_H_
