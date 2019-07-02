/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_GET_TIPS_H_
#define RESPONSE_TA_GET_TIPS_H_

#include <stdlib.h>
#include "utils/containers/hash/hash243_stack.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_get_tips.h
 */

/** struct of ta_get_tips_res_t */
typedef struct {
  /** tips is a 243 long flex trits hash stack. */
  hash243_stack_t tips;
} ta_get_tips_res_t;

/**
 * Allocate memory of ta_get_tips_res_t
 *
 * @return
 * - struct of ta_get_tips_res_t on success
 * - NULL on error
 */
ta_get_tips_res_t* ta_get_tips_res_new();

/**
 * Free memory of ta_get_tips_res_t
 *
 * @param res Data type of ta_get_tips_res_t
 */
void ta_get_tips_res_free(ta_get_tips_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GET_TIPS_H_
