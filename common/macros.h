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

typedef enum mam_protocol_e { MAM_V1 } mam_protocol_t;

#define NUM_TRYTES_MAM_MSG_ID MAM_MSG_ID_SIZE / 3
#define NUM_TRYTES_MAM_NTRU_PK_SIZE MAM_NTRU_PK_SIZE / 3
#define NUM_TRYTES_MAM_PSK_KEY_SIZE MAM_PSK_KEY_SIZE / 3

#define BUILD_BUG_ON_ZERO(e) ((int)(sizeof(struct { int : (-!!(e)); })))
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#define __must_be_array(a) BUILD_BUG_ON_ZERO(__same_type((a), &(a)[0]))
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]) + __must_be_array(arr))

// TODO The temporary default timeout in cache server is 1 week. We should investigate the performance of redis to
// design a better data structure and appropriate timeout period. And we should study the methodology to partially
// release cached data.
#define CACHE_FAILED_TXN_TIMEOUT (7 * 24 * 60 * 60)

#ifdef __cplusplus
}
#endif

#endif  // COMMON_MACROS_H_
