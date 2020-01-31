/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_POW_H_
#define CORE_POW_H_

#include <stdint.h>
#include "common/model/bundle.h"
#include "common/ta_errors.h"
#include "common/trinary/flex_trit.h"
#include "utarray.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/pow.h
 * @brief PoW interface
 * @example test_pow.c
 */

/**
 * Initialize logger
 */
void pow_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int pow_logger_release();

/**
 * Initiate pow module
 */
void pow_init();

/**
 * Stop interacting with pow module
 */
void pow_destroy();

/**
 * Perform PoW and return the result of flex trits
 *
 * @param[in] trits_in Flex trits that does pow
 * @param[in] mwm Maximum weight magnitude
 *
 * @return a flex_trit_t data
 */
flex_trit_t* ta_pow_flex(const flex_trit_t* const trits_in, const uint8_t mwm);

/**
 * Perform PoW to the given bundle
 *
 * @param[in] bundle Bundle that does pow
 * @param[in] trunk Trunk transaction hash
 * @param[in] branch Branch transaction hash
 * @param[in] mwm Maximum weight magnitude
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_pow(const bundle_transactions_t* bundle, const flex_trit_t* const trunk, const flex_trit_t* const branch,
                const uint8_t mwm);

#ifdef __cplusplus
}
#endif

#endif  // CORE_POW_H_
