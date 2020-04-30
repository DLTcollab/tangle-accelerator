/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef TRYTE_BYTE_CONV_H
#define TRYTE_BYTE_CONV_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Convert bytes to trytes
 *
 * @param[in] input The pointer to bytes array
 * @param[in] len Length of bytes array
 * @param[out] output The pointer to output array
 */
void bytes_to_trytes(unsigned char const *const input, uint16_t len, char *output);

/**
 * @brief Convert trytes to bytes
 *
 * @param[in] input The pointer to trytes array
 * @param[in] input_len Length of trytes array
 * @param[out] output The pointer to output array
 */
void trytes_to_bytes(unsigned char const *const input, uint32_t input_len, char *const output);

#ifdef __cplusplus
}
#endif

#endif  // TRYTE_BYTE_CONV_H
