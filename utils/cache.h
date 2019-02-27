#ifndef UTILS_CACHE_H_
#define UTILS_CACHE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cclient/types/types.h"

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
 * Initiate cache module
 *
 * @return
 * - struct of cache_t on success
 * - NULL on error
 */
cache_t* cache_init();

/**
 * Stop interacting with cache module
 *
 * @param cache Data type for Cache module
 *
 * @return NULL
 */
void cache_stop(cache_t** cache);

/**
 * Delete certain key-value store from cache
 *
 * @param[in] cache Data type for Cache module
 * @param[in] key Key string to search
 *
 * @return
 * - 0 on success
 * - 1 on error
 */
int cache_del(const cache_t* const cache, const char* const key);

/**
 * Get key-value store from in-memory cache
 *
 * @param[in] cache Data type for Cache module
 * @param[in] key Key string to search
 * @param[out] res Result of GET key
 *
 * @return
 * - 0 on success
 * - 1 on error
 */
int cache_get(const cache_t* const cache, const char* const key, char* res);

/**
 * Set key-value store into in-memory cache
 *
 * @param[in] cache Data type for Cache module
 * @param[in] key Key string to store
 * @param[in] value Value string to store
 *
 * @return
 * - 0 on success
 * - 1 on error
 */
int cache_set(const cache_t* const cache, const char* const key,
              const char* const value);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_CACHE_H_
