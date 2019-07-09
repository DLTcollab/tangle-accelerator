/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <hiredis/hiredis.h>
#include "cache.h"
#include "utils/logger_helper.h"

#define BR_LOGGER "backend_redis"

/* private data used by cache_t */
typedef struct {
  redisContext* rc;
} connection_private;
#define CONN(c) ((connection_private*)(c.conn))

static cache_t cache;
static bool cache_state;
static logger_id_t br_logger_id;

/*
 * Private functions
 */

void br_logger_init() { br_logger_id = logger_helper_enable(BR_LOGGER, LOGGER_DEBUG, true); }

int br_logger_release() {
  logger_helper_release(br_logger_id);
  if (logger_helper_destroy() != RC_OK) {
    log_critical(br_logger_id, "[%s:%d] Destroying logger failed %s.\n", __func__, __LINE__, BR_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

static status_t redis_del(redisContext* c, const char* const key) {
  status_t ret = SC_OK;
  if (key == NULL) {
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_NULL");
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "DEL %s", key);
  if (!reply->integer) {
    ret = SC_CACHE_FAILED_RESPONSE;
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_FAILED_RESPONSE");
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_get(redisContext* c, const char* const key, char* res) {
  status_t ret = SC_OK;
  if (key == NULL || res[0] != '\0') {
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_NULL");
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "GET %s", key);
  if (reply->type == REDIS_REPLY_STRING) {
    strncpy(res, reply->str, FLEX_TRIT_SIZE_8019);
  } else {
    ret = SC_CACHE_FAILED_RESPONSE;
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_FAILED_RESPONSE");
  }

  freeReplyObject(reply);
  return ret;
}

static status_t redis_set(redisContext* c, const char* const key, const char* const value) {
  status_t ret = SC_OK;
  if (key == NULL || value == NULL) {
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_NULL");
    return SC_CACHE_NULL;
  }

  redisReply* reply = redisCommand(c, "SETNX %b %b", key, FLEX_TRIT_SIZE_243, value, FLEX_TRIT_SIZE_8019);
  if (!reply->integer) {
    ret = SC_CACHE_FAILED_RESPONSE;
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_FAILED_RESPONSE");
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
  if (CONN(cache)->rc) {
    return true;
  }
  return false;
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
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_del(CONN(cache)->rc, key);
}

status_t cache_get(const char* const key, char* res) {
  if (!cache_state) {
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_get(CONN(cache)->rc, key, res);
}

status_t cache_set(const char* const key, const char* const value) {
  if (!cache_state) {
    log_error(br_logger_id, "[%s:%d:%s]\n", __func__, __LINE__, "SC_CACHE_OFF");
    return SC_CACHE_OFF;
  }
  return redis_set(CONN(cache)->rc, key, value);
}
