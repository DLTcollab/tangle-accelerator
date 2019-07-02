/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_GENERATE_ADDRESS_H_
#define RESPONSE_TA_GENERATE_ADDRESS_H_

#include <stdlib.h>
#include "utils/containers/hash/hash243_queue.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file response/ta_generate_address.h
 */

/** struct of ta_generate_address_res_t */
typedef struct {
  /** Transaction address is a 243 long flex trits hash queue. */
  hash243_queue_t addresses;
} ta_generate_address_res_t;

/**
 * Allocate memory of ta_generate_address_res_t
 *
 * @return
 * - struct of ta_generate_address_res_t on success
 * - NULL on error
 */
ta_generate_address_res_t* ta_generate_address_res_new();

/**
 * Free memory of ta_generate_address_res_t
 *
 * @param res Data type of ta_generate_address_res_t
 */
void ta_generate_address_res_free(ta_generate_address_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_GENERATE_ADDRESS_H_
