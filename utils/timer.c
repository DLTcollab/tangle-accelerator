/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "timer.h"
#include "common/logger.h"

#define TIMER_LOGGER "timer"

static logger_id_t logger_id;
static pthread_mutex_t timer_lock = PTHREAD_MUTEX_INITIALIZER;
struct _ta_timer_t {
  pthread_t *thread_id;
  timer_t *timer_id;
};

void timer_logger_init() { logger_id = logger_helper_enable(TIMER_LOGGER, LOGGER_DEBUG, true); }

int timer_logger_release() {
  logger_helper_release(logger_id);
  if (logger_helper_destroy() != RC_OK) {
    ta_log_error("Destroying logger failed %s.\n", TIMER_LOGGER);
    return EXIT_FAILURE;
  }

  return 0;
}

static void handler(union sigval v) {
  ta_timer_t *args = (ta_timer_t *)v.sival_ptr;
  ta_log_error("timer_id: %p expired\n", *(args->timer_id));

  /* Cancel the thread */
  pthread_mutex_lock(&timer_lock);
  pthread_cancel(*(args->thread_id));
  pthread_mutex_unlock(&timer_lock);
}

ta_timer_t *ta_timer_start(const struct itimerspec *timer, void *callback, void *args) {
  pthread_t *thread_id;
  timer_t *timer_id;
  ta_timer_t *ta_timer;

  thread_id = (pthread_t *)malloc(sizeof(pthread_t));
  timer_id = (timer_t *)malloc(sizeof(timer_t));
  ta_timer = (ta_timer_t *)malloc(sizeof(ta_timer_t));
  struct sigevent sev = {.sigev_notify = SIGEV_THREAD,
                         .sigev_signo = SIGALRM,
                         .sigev_value.sival_ptr = (void *)ta_timer,
                         .sigev_notify_function = handler};
  if (thread_id == NULL || timer_id == NULL || ta_timer == NULL) goto cleanup;
  ta_timer->thread_id = thread_id;
  ta_timer->timer_id = timer_id;

  if (timer_create(CLOCK_REALTIME, &sev, timer_id) == -1) {
    ta_log_error("%s\n", "timer_create_failed");
    goto cleanup;
  } else {
    ta_log_debug("Create timer_id: %p\n", *timer_id);
  }

  timer_settime(*timer_id, 0, timer, NULL);
  pthread_create(thread_id, NULL, callback, args);

  return ta_timer;

cleanup:
  free(thread_id);
  free(timer_id);
  free(ta_timer);
  return NULL;
}

status_t ta_timer_stop(ta_timer_t *ta_timer, void **rval) {
  int ret = 0;
  ret = pthread_join(*(ta_timer->thread_id), rval);

  if (ret) {
    ta_log_error("pthread_join failed: %d\n", ret);
    return SC_UTILS_TIMER_ERROR;
  }

  pthread_mutex_lock(&timer_lock);
  ret = timer_delete(*(ta_timer->timer_id));
  free(ta_timer->timer_id);
  free(ta_timer->thread_id);
  free(ta_timer);
  pthread_mutex_unlock(&timer_lock);

  if (ret) {
    ta_log_error("timer_delete failed: %d\n", ret);
    return SC_UTILS_TIMER_ERROR;
  }

  if (*rval == PTHREAD_CANCELED) {
    return SC_UTILS_TIMER_EXPIRED;
  }

  return SC_OK;
}
