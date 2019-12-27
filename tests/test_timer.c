/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include <pthread.h>
#include "test_define.h"
#include "utils/timer.h"

int finite_thread(void *args) {
  long x = *(int *)args;
  pthread_exit((void *)x);
}

int infinite_thread(void *args) {
  long x = *(int *)args;
  while (1) {
    // Cancellation point
    pthread_testcancel();
  }
  pthread_exit((void *)x);
}

void test_timer_finite(void) {
  const struct itimerspec timeout = {.it_interval = {.tv_sec = 0, .tv_nsec = 0},
                                     .it_value = {.tv_sec = 1, .tv_nsec = 0}};
  int *args = (int *)malloc(sizeof(int));
  TEST_ASSERT(args != NULL);
  *args = 7;

  ta_timer_t *timer_id = ta_timer_start(&timeout, finite_thread, args);
  TEST_ASSERT(timer_id != NULL);

  int *rval;
  int ret = ta_timer_stop(timer_id, (void **)&rval);
  TEST_ASSERT(ret == SC_OK);
  TEST_ASSERT((intptr_t)rval == *args);

  free(args);
}

void test_timer_infinite(void) {
  const struct itimerspec timeout = {.it_interval = {.tv_sec = 0, .tv_nsec = 0},
                                     .it_value = {.tv_sec = 1, .tv_nsec = 0}};
  int *args = (int *)malloc(sizeof(int));
  TEST_ASSERT(args != NULL);
  *args = 7;

  ta_timer_t *timer_id = ta_timer_start(&timeout, infinite_thread, args);
  TEST_ASSERT(timer_id != NULL);

  int *rval;
  int ret = ta_timer_stop(timer_id, (void **)&rval);
  TEST_ASSERT(ret == SC_UTILS_TIMER_EXPIRED);

  free(args);
}

int main(void) {
  UNITY_BEGIN();

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  timer_logger_init();
  RUN_TEST(test_timer_finite);
  RUN_TEST(test_timer_infinite);
  timer_logger_release();
  return UNITY_END();
}
