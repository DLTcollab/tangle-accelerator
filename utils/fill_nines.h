/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_FILL_NINES_H_
#define UTILS_FILL_NINES_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "common/model/transaction.h"
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils/fill_nines.h
 * @brief Padding string with 9s to assigned length
 *
 * 9 would be mapped to 0 in the trinary system of IOTA. Therefore, when the legnth of the input string is less than the
 * length of a certain IOTA transaction field, the user can use this function to use 9's as padding and make the input
 * string long enough.
 */

/**
 * @brief Patch input string with nines into assigned length.
 *
 * @param[out] new_str Output patched string
 * @param[in] old_str Input string which needs to be patched
 * @param[in] new_str_len assigned output string length
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
static inline status_t fill_nines(char* new_str, const char* const old_str, size_t new_str_len) {
  if (!new_str || !old_str || new_str_len != NUM_TRYTES_TAG) {
    return SC_NULL;
  }

  int old_str_len = strlen(old_str);
  strncpy(new_str, old_str, old_str_len);

  int diff = new_str_len - old_str_len;
  if (diff) {
    memset((new_str + old_str_len), '9', diff);
  } else {
    return SC_UTILS_WRONG_INPUT_ARG;
  }
  new_str[new_str_len] = '\0';

  return SC_OK;
}

#ifdef __cplusplus
}
#endif

#endif  // UTILS_FILL_NINES_H_
