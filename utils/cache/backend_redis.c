/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <hiredis/hiredis.h>
#include "cache.h"
#include "common/logger.h"

#define BR_LOGGER "backend_redis"

/* private data used by cache_t */
typedef struct {
  redisContext* rc;
} connection_private;
#define CONN(c) ((connection_private*)(c.conn))

static cache_t cache;
static bool cache_state;
static logger_id_t logger_id;

/*
 * Private functions
 */

void br_logger_init() { logger_id = logger_helper_enable(BR_LOGGER, LOGGER_DEBUG, true); }

int br_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", BR_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

static status_t redis_del(redisContext* c, const char* const key) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "DEL %s", key);
  if (!reply->integer) {
    ret = SC_CACHE_FAILED_RESPONSE;
    ta_log_error("%s\n", "SC_CACHE_FAILED_RESPONSE");
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_get(redisContext* c, const char* const key, char* res) {
  status_t ret = SC_OK;
  if (key == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "GET %s", key);
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

static status_t redis_set(redisContext* c, const char* const key, const int key_size, const void* const value,
                          const int value_size, const int timeout) {
  status_t ret = SC_OK;
  if (c == NULL || key == NULL || value == NULL) {
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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
    ta_log_error("%s\n", ta_error_to_string(SC_CACHE_NULL));
    return SC_CACHE_NULL;
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

/*
 * Public functions
 */

bool cache_init(bool state, const char* host, int port) {
  cache_state = state;
  if (!state) {
    return false;
  }

  cache.conn = (connection_private*)malloc(sizeof(connection_private));
  CONN(cache)->rc = redisConnect(host, port);
  if (!CONN(cache)->rc || CONN(cache)->rc->err) {
    ta_log_error("Failed to initialize redis: %s\n", CONN(cache)->rc->err);
    return false;
  }
  return true;
}

void cache_stop() {
  if (cache_state == true && CONN(cache)->rc) {
    redisFree(CONN(cache)->rc);

    if (CONN(cache)) {
      free(CONN(cache));
    }
  }
}

status_t cache_del(const char* const key) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_del(CONN(cache)->rc, key);
}

status_t cache_get(const char* const key, char* res) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_get(CONN(cache)->rc, key, res);
}

status_t cache_set(const char* const key, const int key_size, const void* const value, const int value_size,
                   const int timeout) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_set(CONN(cache)->rc, key, key_size, value, value_size, timeout);
}

status_t cache_list_push(const char* const key, const int key_size, const void* const value, const int value_size) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_list_push(CONN(cache)->rc, key, key_size, value, value_size);
}

status_t cache_list_at(const char* const key, const int index, const int res_len, char* res) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_list_at(CONN(cache)->rc, key, index, res_len, res);
}

status_t cache_list_peek(const char* const key, const int res_len, char* res) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_list_peek(CONN(cache)->rc, key, res_len, res);
}

status_t cache_list_size(const char* const key, int* len) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_list_size(CONN(cache)->rc, key, len);
}

status_t cache_list_exist(const char* const key, const char* const value, const int value_len, bool* exist) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_list_exist(CONN(cache)->rc, key, value, value_len, exist);
}

status_t cache_list_pop(const char* const key, char* res) {
  if (!cache_state) {
    ta_log_info("%s\n", "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_list_pop(CONN(cache)->rc, key, res);
}
