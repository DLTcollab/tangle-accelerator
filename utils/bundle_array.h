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
#include "common/ta_errors.h"
#include "utarray.h"

/**
 * @file utils/bundle_array.h
 * @brief Implementation of bundle array object. This object would be used when we fetch multiple
 * `bundle_transactions_t` objects. It provides an easier way to save and traverse all the bundles.
 */

/**
 * @brief Renew the given bundle
 *
 * @param bundle[in,out] The bundle that will be renewed
 *
 */
static inline void bundle_transactions_renew(bundle_transactions_t **bundle) {
  bundle_transactions_free(bundle);
  bundle_transactions_new(bundle);
}

typedef UT_array bundle_array_t;
// We should synchronize this implementation as to the implementation in the IOTA library
static UT_icd bundle_transactions_icd = {sizeof(iota_transaction_t), 0, 0, 0};

static inline void bundle_array_copy(void *_dst, const void *_src) {
  bundle_transactions_t *bdl_dst = (bundle_transactions_t *)_dst;
  bundle_transactions_t *bdl_src = (bundle_transactions_t *)_src;
  iota_transaction_t *txn = NULL;
  utarray_init(bdl_dst, &bundle_transactions_icd);
  BUNDLE_FOREACH(bdl_src, txn) { bundle_transactions_add(bdl_dst, txn); }
}

static UT_icd bundle_array_icd = {sizeof(bundle_transactions_t), 0, bundle_array_copy, 0};

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
 * @brief Gets the number of bundles in the bundle_array.
 *
 * @param[in] bundle_array The bundle array object.
 * @return An number of bundles.
 */
static inline size_t bundle_array_size(bundle_array_t const *const bundle_array) {
  if (bundle_array == NULL) {
    return 0;
  }

  return utarray_len(bundle_array);
}

/**
 * @brief Adds a bundle to the bundle_array.
 *
 * @param[in] bundle_array The bundle array object.
 * @param[in] bundle A bundle object.
 * @return #retcode_t
 */
static inline status_t bundle_array_add(bundle_array_t *const bundle_array, bundle_transactions_t const *const bundle) {
  if (!bundle || !bundle_array) {
    return SC_TA_NULL;
  }

  utarray_push_back(bundle_array, bundle);
  return SC_OK;
}

/**
 * @brief Gets a bundle from the bundle_array by index.
 *
 * @param[in] bundle_array The bundle array object.
 * @param[in] index The index of a bundle.
 * @return #bundle_transactions_t
 */
static inline bundle_transactions_t *bundle_array_at(bundle_array_t *const bundle_array, size_t index) {
  if (index < utarray_len(bundle_array)) {
    return (bundle_transactions_t *)(utarray_eltptr(bundle_array, index));
  }
  return NULL;
}

/**
 * Free memory of bundle_array_t
 */
static inline void bundle_array_free(bundle_array_t **const bundle_array) {
  // TODO set dtor and use utarray_free() instead.
  bundle_transactions_t *bundle = NULL;
  BUNDLE_ARRAY_FOREACH(*bundle_array, bundle) { utarray_done(bundle); }
  utarray_free(*bundle_array);
  *bundle_array = NULL;
}

#ifdef __cplusplus
}
#endif

#endif  // UTILS_BUNDLE_ARRAY_H_
