/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
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
#include "accelerator/errors.h"
#include "common/trinary/flex_trit.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file cache.h
 * @brief Implementation of cache interface
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
 * @param[in] value Value string to store
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t cache_set(const char* const key, const char* const value);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_CACHE_H_
