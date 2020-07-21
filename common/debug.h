/*
 * Copyright (c) 2018 IOTA Stiftung
 * https://github.com/iotaledger/entangled
 *
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef COMMON_DEBUG_H_
#define COMMON_DEBUG_H_

#include "common/crypto/iss/v1/iss_kerl.h"
#include "common/helpers/sign.h"
#include "common/model/bundle.h"
#include "common/trinary/trit_long.h"
#include "common/trinary/tryte_long.h"

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common/crypto/curl-p/digest.h"
#include "common/model/transaction.h"
#include "common/trinary/trit_long.h"

#include "logger.h"

/**
 * @file common/debug.h
 * @brief Debugging tool for tangle-accelerator
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Initialize logger
 */
void debug_logger_init();

/**
 * @brief Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int debug_logger_release();

/**
 * @brief Print the content of all the transaction objects in this bundle
 *
 * @param[in] bundle Bundle is going to dump message.
 *
 */
void dump_bundle(bundle_transactions_t *bundle);

/**
 * @brief Print the all the contents this transaction object
 *
 * @param[in] tx_obj Bundle is going to dump message.
 *
 */
void dump_transaction_obj(iota_transaction_t *tx_obj);

/**
 * @brief Compare two transaction objects
 *
 * @param[in] tx1 Compared transaction object one
 * @param[in] tx2 Compared transaction object two
 *
 * @return
 * - true on success
 * - false on error
 */
bool transaction_cmp(iota_transaction_t *tx1, iota_transaction_t *tx2);

#ifdef __cplusplus
}
#endif

#endif  // COMMON_DEBUG_H_
