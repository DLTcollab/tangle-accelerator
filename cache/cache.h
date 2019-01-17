#ifndef CACHE_CACHE_H_
#define CACHE_CACHE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "third_party/hiredis/hiredis.h"

typedef struct {
  void* conn;
} cache_t;

/** Initiate cache module */
cache_t* cache_init();

/** Stop interacting with cache module */
void cache_stop(cache_t** cache);

/**
 * Delete certain key-value store from cache
 *
 * @param cache Data type for Cache module
 * @param key Key string to search
 *
 * @return {int} Status code
 */
int cache_del(const cache_t* const cache, const char* const key);

/**
 * Get key-value store from in-memory cache
 *
 * @param cache Data type for Cache module
 * @param key Key string to search
 *
 * @return {string} res Result of GET key
 * @return {int} Status code
 */
int cache_get(const cache_t* const cache, const char* const key, char* res);

/**
 * Set key-value store into in-memory cache
 *
 * @param cache Data type for Cache module
 * @param key Key string to store
 * @param value Value string to store
 *
 * @return {int} Status code
 */
int cache_set(const cache_t* const cache, const char* const key,
              const char* const value);

#ifdef __cplusplus
}
#endif

#endif  // CACHE_CACHE_H_
