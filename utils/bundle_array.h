/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_BUNDLE_ARRAY_H_
#define UTILS_BUNDLE_ARRAY_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "common/model/bundle.h"
#include "utarray.h"

/**
 * @file utils/bundle_array.h
 * @brief Implementation of bundle array object. This object would be used when we fetch multiple
 * `bundle_transactions_t` objects. It provides an easier way to save and traverse all the bundles.
 */

typedef UT_array bundle_array_t;

static UT_icd bundle_array_icd = {sizeof(bundle_transactions_t), 0, 0, 0};

/**
 * Allocate memory of bundle_array_t
 */
static inline void bundle_array_new(bundle_array_t **const bundle_array) {
  utarray_new(*bundle_array, &bundle_array_icd);
}

/**
 * @brief The bundle array iterator.
 */
#define BUNDLE_ARRAY_FOREACH(bundles, bdl)                                 \
  for (bdl = (bundle_transactions_t *)utarray_front(bundles); bdl != NULL; \
       bdl = (bundle_transactions_t *)utarray_next(bundles, bdl))

/**
 * Free memory of bundle_array_t
 */
static inline void bundle_array_free(bundle_array_t **const bundle_array) {
  // TODO set dtor and use utarray_free() instead.
  if (bundle_array && *bundle_array) {
    bundle_transactions_t *bundle = NULL;
    BUNDLE_ARRAY_FOREACH(*bundle_array, bundle) { bundle_transactions_free(&bundle); }
    free(*bundle_array);
  }
  *bundle_array = NULL;
}

#ifdef __cplusplus
}
#endif

#endif  // UTILS_BUNDLE_ARRAY_H_
