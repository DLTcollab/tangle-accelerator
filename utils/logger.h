/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_LOGGER_H_
#define ACCELERATOR_LOGGER_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "utils/logger_helper.h"

static inline bool ta_logger_init() {
#ifdef NDEBUG
  if (logger_helper_init(LOGGER_INFO) != RC_OK) {
    return false;
  }
#else
  if (logger_helper_init(LOGGER_DEBUG) != RC_OK) {
    return false;
  }
#endif
  return true;
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

bool verbose_mode; /**< flag of verbose mode */
/*
#define ta_logger_declare(module, level) \
  void module##_logger_init() { logger_id = logger_helper_enable(#module, level, true); } \
  int module##_logger_release() {
    logger_helper_release
  }
*/

#ifdef __cplusplus
}
#endif

#endif  // TA_SCYLLA_API_H_
