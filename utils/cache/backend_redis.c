/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <errno.h>
#include <hiredis/hiredis.h>
#include <limits.h>
#include "cache.h"
#include "common/logger.h"

#define BR_LOGGER "backend_redis"

/* private data used by cache_t */
typedef struct {
  redisContext* rc;
} connection_private;
#define CONN(c) ((connection_private*)(c.conn))

static cache_t cache;
static bool state = false;
static logger_id_t logger_id;

/*
 * Private functions
 */

void br_logger_init() { logger_id = logger_helper_enable(BR_LOGGER, LOGGER_DEBUG, true); }

int br_logger_release() {
  logger_helper_release(logger_id);
  return 0;
}

static status_t redis_del(redisContext* c, const char* const key) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = redisCommand(c, "DEL %s", key);
  if (!reply->integer) {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CACHE_FAILED_RESPONSE");
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_get(redisContext* c, const char* const key, char** res) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }
  *res = NULL;

  redisReply* reply = redisCommand(c, "GET %s", key);
  if (reply->type == REDIS_REPLY_STRING) {
    *res = strdup(reply->str);
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_set(redisContext* c, const char* const key, const int key_size, const void* const value,
                          const int value_size, const int timeout) {
  status_t ret = SC_OK;
  if (c == NULL || key == NULL || value == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = NULL;
  if (timeout > 0) {
    reply = redisCommand(c, "SET %b %b EX %d NX", key, key_size, value, value_size, timeout);
  } else {
    reply = redisCommand(c, "SET %b %b NX", key, key_size, value, value_size);
  }

  if (reply->type != REDIS_REPLY_STATUS) {
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_list_push(redisContext* c, const char* const key, const int key_size, const void* const value,
                                const int value_size) {
  status_t ret = SC_OK;
  if (c == NULL || key == NULL || value == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = NULL;
  reply = redisCommand(c, "LPUSH %b %b", key, key_size, value, value_size);
  if (reply->type == REDIS_REPLY_ERROR) {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_list_at(redisContext* c, const char* const key, const int index, const int res_len, char* res) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = redisCommand(c, "LINDEX %s %d", key, index);
  if (reply->type == REDIS_REPLY_STRING && ((int)reply->len) <= res_len) {
    strncpy(res, reply->str, reply->len);
    res[reply->len] = 0;
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_list_peek(redisContext* c, const char* const key, const int res_len, char* res) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = redisCommand(c, "LINDEX %s %d", key, 0);
  if (reply->type == REDIS_REPLY_STRING && ((int)reply->len) <= res_len) {
    strncpy(res, reply->str, reply->len);
    res[reply->len] = 0;
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_list_size(redisContext* c, const char* const key, int* len) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = redisCommand(c, "LLEN %s", key);
  if (reply->type != REDIS_REPLY_ERROR) {
    // We constrain the length should less than the maximun of `int`
    *len = (int)reply->integer;
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_list_exist(redisContext* c, const char* const key, const char* const value, const int value_len,
                                 bool* exist) {
  status_t ret = SC_OK;
  if (key == NULL || value == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }
  char res[value_len + 1];
  int len = 0;
  *exist = false;

  if (redis_list_size(c, key, &len) != SC_OK) {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  for (int i = 0; i < len; ++i) {
    if (redis_list_at(c, key, i, value_len, res) == SC_OK) {
      if (!memcmp(value, res, value_len)) {
        *exist = true;
      }
    } else {
      ret = SC_CACHE_FAILED_RESPONSE;
      ta_log_error("%s\n", ta_error_to_string(ret));
    }
  }

  return ret;
}

static status_t redis_list_pop(redisContext* c, const char* const key, char* res) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_NULL));
    return SC_NULL;
  }

  redisReply* reply = redisCommand(c, "LPOP %s", key);
  if (reply->type == REDIS_REPLY_STRING) {
    strncpy(res, reply->str, reply->len);
    res[reply->len] = 0;
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", ta_error_to_string(ret));
  }

  freeReplyObject(reply);
  return ret;
}

long int redis_occupied_space(redisContext* c) {
  const char size_field[] = "used_memory:";
  redisReply* reply = redisCommand(c, "INFO");
  if (reply->type == REDIS_REPLY_STRING) {
    // Parsing the result returned by redis. Get information from field `used_memory`
    char* str = strstr(reply->str, size_field);
    char* size_str = strtok(str, "\n");
    char* ptr = size_str + strlen(size_field);

    char* strtol_p = NULL;
    long int strtol_temp;
    strtol_temp = strtol(ptr, &strtol_p, 10);
    if (strtol_p != ptr && errno != ERANGE && strtol_temp >= INT_MIN && strtol_temp <= INT_MAX) {
      freeReplyObject(reply);
      return strtol_temp;
    } else {
      ta_log_error("Malformed input\n");
    }
  } else {
    ta_log_error("%s\n", "Failed to operate redis command.");
  }

  freeReplyObject(reply);
  return -1;
}

void redis_set_capacity(redisContext* c, char* size) {
  redisReply* reply = redisCommand(c, "CONFIG SET maxmemory %s", size);
  printf("%s\n", reply->str);
}

void redis_get_capacity(redisContext* c) {
  redisReply* reply = redisCommand(c, "CONFIG GET maxmemory");
  printf("%s\n", reply->element[1]->str);
}

/*
 * Public functions
 */

bool cache_init(pthread_rwlock_t** rwlock, bool input_state, const char* host, int port) {
  state = input_state;
  if (!state) {
    ta_log_error("Caching service is not enabled.\n");
    return false;
  }

  cache.conn = (connection_private*)malloc(sizeof(connection_private));
  CONN(cache)->rc = redisConnect(host, port);
  if (!CONN(cache)->rc || CONN(cache)->rc->err) {
    ta_log_error("Failed to initialize redis: %d\n", CONN(cache)->rc->err);
    return false;
  }

  *rwlock = (pthread_rwlock_t*)malloc(sizeof(pthread_rwlock_t));
  if (pthread_rwlock_init(*rwlock, NULL)) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_INIT_FINI));
    return SC_CACHE_INIT_FINI;
  }

  return true;
}

void cache_stop(pthread_rwlock_t** rwlock) {
  if (state == true && CONN(cache)->rc) {
    redisFree(CONN(cache)->rc);

    if (pthread_rwlock_destroy(*rwlock)) {
      ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_INIT_FINI));
    }
    free(*rwlock);

    if (CONN(cache)) {
      free(CONN(cache));
    }
  }
}

status_t cache_del(const char* const key) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_del(CONN(cache)->rc, key);
}

status_t cache_get(const char* const key, char** res) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_get(CONN(cache)->rc, key, res);
}

status_t cache_set(const char* const key, const int key_size, const void* const value, const int value_size,
                   const int timeout) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_set(CONN(cache)->rc, key, key_size, value, value_size, timeout);
}

status_t cache_list_push(const char* const key, const int key_size, const void* const value, const int value_size) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_list_push(CONN(cache)->rc, key, key_size, value, value_size);
}

status_t cache_list_at(const char* const key, const int index, const int res_len, char* res) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_list_at(CONN(cache)->rc, key, index, res_len, res);
}

status_t cache_list_peek(const char* const key, const int res_len, char* res) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_list_peek(CONN(cache)->rc, key, res_len, res);
}

status_t cache_list_size(const char* const key, int* len) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_list_size(CONN(cache)->rc, key, len);
}

status_t cache_list_exist(const char* const key, const char* const value, const int value_len, bool* exist) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_list_exist(CONN(cache)->rc, key, value, value_len, exist);
}

status_t cache_list_pop(const char* const key, char* res) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }
  return redis_list_pop(CONN(cache)->rc, key, res);
}

long int cache_occupied_space() {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }

  return redis_occupied_space(CONN(cache)->rc);
}

long int cache_set_capacity(char* size) {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }

  redis_set_capacity(CONN(cache)->rc, size);
  return 1;
}

long int cache_get_capacity() {
  if (!state) {
    ta_log_debug("%s\n", ta_error_to_string(SC_CACHE_OFF));
    return SC_CACHE_OFF;
  }

  redis_get_capacity(CONN(cache)->rc);
  return 1;
}
