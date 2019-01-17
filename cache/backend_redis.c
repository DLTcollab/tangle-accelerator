#include "./config.h"
#include "cache.h"

/* private data used by cache_t */
typedef struct {
  redisContext* rc;
} connection_private;
#define CONN(c) ((connection_private*)(c->conn))

/*
 * Private functions
 */

static int redis_del(redisContext* c, const char* const key) {
  int ret = 1;

  if (key == NULL) {
    return ret;
  }

  redisReply* reply = redisCommand(c, "DEL %s", key);
  if (reply->integer) {
    ret = 0;
  }

  freeReplyObject(reply);
  return ret;
}

static int redis_get(redisContext* c, const char* const key, char* res) {
  int ret = 1;

  if (key == NULL || res[0] != '\0') {
    return ret;
  }

  redisReply* reply = redisCommand(c, "GET %s", key);
  if (reply->type == REDIS_REPLY_STRING) {
    strcpy(res, reply->str);
    ret = 0;
  }

  freeReplyObject(reply);
  return ret;
}

static int redis_set(redisContext* c, const char* const key,
                     const char* const value) {
  int ret = 1;

  if (key == NULL || value == NULL) {
    return ret;
  }

  redisReply* reply = redisCommand(c, "SETNX %s %s", key, value);
  if (reply->integer) {
    ret = 0;
  }

  freeReplyObject(reply);
  return ret;
}

/*
 * Public functions
 */

cache_t* cache_init() {
  cache_t* cache = (cache_t*)malloc(sizeof(cache_t));
  if (cache != NULL) {
    cache->conn = (connection_private*)malloc(sizeof(connection_private));
    CONN(cache)->rc = redisConnect(REDIS_HOST, REDIS_PORT);
    return cache;
  }
  return NULL;
}

void cache_stop(cache_t** cache) {
  if (*cache) {
    redisFree(CONN((*cache))->rc);

    if (CONN((*cache))) {
      free(CONN((*cache)));
    }

    free(*cache);
    *cache = NULL;
  }
}

int cache_del(const cache_t* const cache, const char* const key) {
  return redis_del(CONN(cache)->rc, key);
}

int cache_get(const cache_t* const cache, const char* const key, char* res) {
  return redis_get(CONN(cache)->rc, key, res);
}

int cache_set(const cache_t* const cache, const char* const key,
              const char* const value) {
  return redis_set(CONN(cache)->rc, key, value);
}
