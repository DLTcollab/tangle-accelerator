/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_TIMER_H_
#define UTILS_TIMER_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils/timer.h
 * @brief Implementation of one-shot timer.
 *
 * The wrapper wraps and executes the callback, executes in a different thread, and cancels the thread after the given
 * timeout.
 */

#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include "common/ta_errors.h"

/**
 * Initialize logger
 */
void timer_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int timer_logger_release();

typedef struct _ta_timer_t ta_timer_t;

/**
 * Create and initialize a ta_timer
 * @param[in] timer Timeout to wait until callback being halted
 * @param[in] callback The actual function being called
 * @param[in] args The arguments to pass to the callback function
 *
 * @return
 * - ta_timer pointer
 * - NULL on error
 */
ta_timer_t *ta_timer_start(const struct itimerspec *timer, void *callback, void *args);

/**
 * Stop a timer and block the current thread until timeout or callback finishes.
 * @param[in] ta_timer timer obtained by ta_timer_start
 * @param[out] rval Return value of the callback
 *
 * @return
 * - SC_OK on success
 * - non zero on error
 */
status_t ta_timer_stop(ta_timer_t *ta_timer, void **rval);

#ifdef __cplusplus
}
#endif

#endif  // UTILS_TIMER_H_
