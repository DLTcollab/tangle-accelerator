/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
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
#define ta_log_debug(args...)                               \
  do {                                                      \
    log_debug(logger_id, "[%s : %d] ", __func__, __LINE__); \
    log_debug(logger_id, ##args);                           \
    fflush(stdout);                                         \
  } while (0)
#define ta_log_info(args...)                               \
  do {                                                     \
    log_info(logger_id, "[%s : %d] ", __func__, __LINE__); \
    log_info(logger_id, ##args);                           \
    fflush(stdout);                                        \
  } while (0)
#define ta_log_warning(args...)                               \
  do {                                                        \
    log_warning(logger_id, "[%s : %d] ", __func__, __LINE__); \
    log_warning(logger_id, ##args);                           \
    fflush(stdout);                                           \
  } while (0)
#define ta_log_error(args...)                               \
  do {                                                      \
    log_error(logger_id, "[%s : %d] ", __func__, __LINE__); \
    log_error(logger_id, ##args);                           \
    fflush(stdout);                                         \
  } while (0)

bool quiet_mode; /**< flag of quiet mode */

#ifdef __cplusplus
}
#endif

#endif  // COMMON_LOGGER_H_
