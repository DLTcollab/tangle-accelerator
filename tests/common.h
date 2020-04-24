/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <stdio.h>
#include <time.h>
#include "accelerator/config.h"
#include "common/ta_errors.h"

#define TXN_HASH_NUM 2

typedef struct driver_test_cases_s {
  char* txn_hash[TXN_HASH_NUM];
  char* tag;
} driver_test_cases_t;

/**
 * Set option list for `get_long()`
 *
 * @return
 * - pointer of a `struct option` array on success
 * - NULL on error
 */
struct option* driver_cli_build_options();

/**
 * Initializes configurations with default values for driver tests
 *
 * @param core[in] Pointer to Tangle-accelerator core configuration structure
 * @param argc[in] Pointer to Tangle-accelerator core configuration structure
 * @param argv[in] Pointer to Tangle-accelerator core configuration structure
 * @param test_cases[in] Pointer to Tangle-accelerator core configuration structure
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t driver_core_cli_init(ta_core_t* const core, int argc, char** argv, driver_test_cases_t* test_cases);

/**
 * Start testing clock
 *
 * @param start[out] `struct timespec` object for recording starting time.
 *
 */
static inline void test_time_start(struct timespec* start) { clock_gettime(CLOCK_REALTIME, start); }

/**
 * Initializes configurations with default values
 *
 * @param start[in] `struct timespec` object for recording starting time.
 * @param end[out] `struct timespec` object for recording ending time.
 * @param start[out] The total elapsing time.
 *
 */
void test_time_end(struct timespec* start, struct timespec* end, double* sum);

/**
 * Generate a random trytes combination with given length
 *
 * @param len[in] The length of generated trytes
 * @param trytes[out] Output trytes combination
 *
 */
void gen_rand_trytes(int len, tryte_t* trytes);
