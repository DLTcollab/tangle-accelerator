/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef COMMON_MACROS_H_
#define COMMON_MACROS_H_

#include "common/model/transaction.h"
#include "common/trinary/tryte.h"
#include "mam/mam/message.h"

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file common/ta_macros.h
 * @brief Macros for tangle-accelerator
 */

#define NUM_TRYTES_MAM_MSG_ID MAM_MSG_ID_SIZE / 3
#define NUM_TRYTES_MAM_NTRU_PK_SIZE MAM_NTRU_PK_SIZE / 3
#define NUM_TRYTES_MAM_PSK_KEY_SIZE MAM_PSK_KEY_SIZE / 3

#ifdef __cplusplus
}
#endif

#endif  // COMMON_MACROS_H_
