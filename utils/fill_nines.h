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
#include "accelerator/errors.h"
#include "common/model/transaction.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils/fill_nines.h
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
status_t fill_nines(char* new_str, const char* const old_str, size_t new_str_len);
#ifdef __cplusplus
}
#endif

#endif  // UTILS_FILL_NINES_H_
