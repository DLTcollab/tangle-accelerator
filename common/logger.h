/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef COMMON_LOGGER_H_
#define COMMON_LOGGER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "common/ta_errors.h"
#include "utils/logger_helper.h"

#ifdef NDEBUG
#define TA_LOGGER_LEVEL LOGGER_INFO
#else
#define TA_LOGGER_LEVEL LOGGER_DEBUG
#endif

/**
 * @file common/logger.h
 */

/**
 * @brief initialize logger level according to build type.
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static inline status_t ta_logger_init() {
  if (logger_helper_init(TA_LOGGER_LEVEL) != RC_OK) {
    return SC_TA_LOGGER_INIT_FAIL;
  }
  return SC_OK;
}

/*
 * logger's wrapper (sorted by priority)
 *   - debug : Debug messages only show when debug build and can be used in development or troubleshooting.
 *   - info  : Information about programs' progress or useful information to users.
 *   - warning : Potential problems or failure which users should care about.
 *   - error : Error events prevent programs to execute normally.
 */
#define ta_log_debug(fmt, args...)                                      \
  do {                                                                  \
    log_debug(logger_id, "[%s : %d] " fmt, __func__, __LINE__, ##args); \
    fflush(stdout);                                                     \
  } while (0)
#define ta_log_info(fmt, args...)                                      \
  do {                                                                 \
    log_info(logger_id, "[%s : %d] " fmt, __func__, __LINE__, ##args); \
    fflush(stdout);                                                    \
  } while (0)
#define ta_log_warning(fmt, args...)                                      \
  do {                                                                    \
    log_warning(logger_id, "[%s : %d] " fmt, __func__, __LINE__, ##args); \
    fflush(stdout);                                                       \
  } while (0)
#define ta_log_error(fmt, args...)                                      \
  do {                                                                  \
    log_error(logger_id, "[%s : %d] " fmt, __func__, __LINE__, ##args); \
    fflush(stdout);                                                     \
  } while (0)

#ifdef MQTT_ENABLE
/**
 * @brief Initialize MQTT utils logger
 *
 * This function is implemented in connectivity/mqtt/duplex_utils.c
 */
void mqtt_utils_logger_init();
/**
 * @brief Release MQTT utils logger
 *
 * This function is implemented in connectivity/mqtt/duplex_utils.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int mqtt_utils_logger_release();

/**
 * @brief Initialize MQTT common logger
 *
 * This function is implemented in connectivity/mqtt/mqtt_common.c
 */
void mqtt_common_logger_init();
/**
 * @brief Release MQTT common logger
 *
 * This function is implemented in connectivity/mqtt/mqtt_common.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int mqtt_common_logger_release();

/**
 * @brief Initialize MQTT callback logger
 *
 * This function is implemented in connectivity/mqtt/duplex_callback.c
 */
void mqtt_callback_logger_init();
/**
 * @brief Release MQTT callback logger
 *
 * This function is implemented in connectivity/mqtt/duplex_callback.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int mqtt_callback_logger_release();

/**
 * @brief Initialize MQTT pub logger
 *
 * This function is implemented in connectivity/mqtt/pub_utils.c
 */
void mqtt_pub_logger_init();
/**
 * @brief Release MQTT pub logger
 *
 * This function is implemented in connectivity/mqtt/pub_utils.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int mqtt_pub_logger_release();

/**
 * @brief Initialize MQTT sub logger
 *
 * This function is implemented in connectivity/mqtt/sub_utils.c
 */
void mqtt_sub_logger_init();
/**
 * @brief Release MQTT sub logger
 *
 * This function is implemented in connectivity/mqtt/sub_utils.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int mqtt_sub_logger_release();
#else
/**
 * @brief Initialize http logger
 *
 * This function is implemented in connectivity/http/http.c
 */
void http_logger_init();
/**
 * @brief Release conn logger
 *
 * This function is implemented in connectivity/http/http.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int http_logger_release();
#endif /* MQTT_ENABLE */

/**
 * @brief Initialize conn logger
 *
 * This function is implemented in connectivity/common.c
 */
void conn_logger_init();
/**
 * @brief Release conn logger
 *
 * This function is implemented in connectivity/common.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int conn_logger_release();

/**
 * @brief Initialize apis logger
 *
 * This function is implemented in accelerator/core/apis.c
 */
void apis_logger_init();

/**
 * @brief Release apis logger
 *
 * This function is implemented in accelerator/core/apis.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int apis_logger_release();

/**
 * @brief Initialize core logger
 *
 * This function is implemented in accelerator/core/core.c
 */
void cc_logger_init();

/**
 * @brief Release logger
 *
 * This function is implemented in accelerator/core/core.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int cc_logger_release();

/**
 * @brief Initialize serializer logger
 *
 * This function is implemented in accelerator/core/serializer/serializer.c
 */
void serializer_logger_init();

/**
 * @brief Release serializer logger
 *
 * This function is implemented in accelerator/core/serializer/serializer.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int serializer_logger_release();

/**
 * @brief Initialize PoW logger
 *
 * This function is implemented in accelerator/core/pow.c
 */
void pow_logger_init();

/**
 * @brief Release PoW logger
 *
 * This function is implemented in accelerator/core/pow.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int pow_logger_release();

/**
 * @brief Initialize timer logger
 *
 * This function is implemented in utils/timer.c
 */
void timer_logger_init();
/**
 * @brief Release timer logger
 *
 * This function is implemented in utils/timer.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int timer_logger_release();

/**
 * @brief Initialize backend_redis logger
 *
 * This function is implemented in utils/cache/backend_redis.c
 */
void br_logger_init();

/**
 * @brief Release logger
 *
 * This function is implemented in utils/cache/backend_redis.c
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int br_logger_release();

/**
 * Initialize logger for ECDH
 */
void ecdh_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int ecdh_logger_release();

#ifdef __cplusplus
}
#endif

#endif  // COMMON_LOGGER_H_
