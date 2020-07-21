/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_CHAR_BUFFER_STR_H_
#define UTILS_CHAR_BUFFER_STR_H_

#include <stdlib.h>
#include <string.h>
#include "common/ta_errors.h"
#include "utils/char_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils/char_buffer_str.h
 * @brief Get string from 'char_buffer_t' object.
 */

/**
 * @brief Get string from char_buffer_t
 * @param[in] char_buff char_buffer_t object
 * @param[out] json_result Response message
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static inline status_t str_from_char_buffer(char_buffer_t* char_buff, char** json_result) {
  if (char_buff == NULL) {
    return SC_NULL;
  }

  *json_result = strdup(char_buff->data);
  if (*json_result == NULL) {
    return SC_OOM;
  }

  return SC_OK;
}

#ifdef __cplusplus
}
#endif

#endif  // UTILS_CHAR_BUFFER_STR_H_
