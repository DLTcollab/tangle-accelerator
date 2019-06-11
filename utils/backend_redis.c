#include <hiredis/hiredis.h>
#include "cache.h"

/* private data used by cache_t */
typedef struct {
  redisContext* rc;
} connection_private;
#define CONN(c) ((connection_private*)(c.conn))

static cache_t cache;

/*
 * Private functions
 */

static status_t redis_del(redisContext* c, const char* const key) {
  status_t ret = SC_OK;

  if (key == NULL) {
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "DEL %s", key);
  if (!reply->integer) {
    ret = SC_CACHE_FAILED_RESPONSE;
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_get(redisContext* c, const char* const key, char* res) {
  status_t ret = SC_OK;

  if (key == NULL || res[0] != '\0') {
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "GET %s", key);
  if (reply->type == REDIS_REPLY_STRING) {
    strncpy(res, reply->str, FLEX_TRIT_SIZE_8019);
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_set(redisContext* c, const char* const key, const char* const value) {
  status_t ret = SC_OK;

  if (key == NULL || value == NULL) {
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "SETNX %b %b", key, FLEX_TRIT_SIZE_243, value, FLEX_TRIT_SIZE_8019);
  if (!reply->integer) {
    ret = SC_CACHE_FAILED_RESPONSE;
  }

  freeReplyObject(reply);
  return ret;
}

/*
 * Public functions
 */

bool cache_init(const char* host, int port) {
  cache.conn = (connection_private*)malloc(sizeof(connection_private));
  CONN(cache)->rc = redisConnect(host, port);
  if (CONN(cache)->rc) {
    return true;
  }
  return false;
}

void cache_stop() {
  if (CONN(cache)->rc) {
    redisFree(CONN(cache)->rc);

    if (CONN(cache)) {
      free(CONN(cache));
    }
  }
}

status_t cache_del(const char* const key) { return redis_del(CONN(cache)->rc, key); }

status_t cache_get(const char* const key, char* res) { return redis_get(CONN(cache)->rc, key, res); }

status_t cache_set(const char* const key, const char* const value) { return redis_set(CONN(cache)->rc, key, value); }
