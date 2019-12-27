/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#ifndef UTILS_HASH_ALGO_DJB2_H_
#define UTILS_HASH_ALGO_DJB2_H_

#ifdef __cplusplus
extern "C" {
#endif

// source http://www.cse.yorku.ca/~oz/hash.html
static inline uint32_t hash_algo_djb2(char const* str) {
  uint32_t hash = 5381;
  int c;

  while ((c = *str++)) {
    hash = ((hash << 5) + hash) + c;
  }

  return hash;
}

#ifdef __cplusplus
}
#endif

#endif  // UTILS_HASH_ALGO_DJB2_H_
