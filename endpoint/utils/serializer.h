/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SERIALIZER_H
#define SERIALIZER_H

#include <stdint.h>
#include "defined_error.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Serialize ciphertext and initialize vector together, the out put message
 * format show as below:
 * - initialize vector(16 bytes)
 * - ciphertext length(10 bytes)
 * - ciphertext(other bytest)
 *
 * @param[in] iv Pointer to initialize vector
 * @param[in] ciphertext_len Length of ciphertext
 * @param[in] ciphertext Ciphertext to be serialized
 * @param[out] out_msg Pointer to output message
 * @param[out] out_msg_len Pointer to length of serialized message
 *
 * @return
 * - RET_OK on success
 * - RET_FAULT on error
 */
retcode_t serialize_msg(const uint8_t *iv, uint32_t ciphertext_len, const char *ciphertext, char *out_msg,
                        uint32_t *out_msg_len);

/**
 * @brief Deserialize message from serialize_msg
 *
 * @param[in] msg Pointer to serialize message
 * @param[in] iv Pointer to initialize vector
 * @param[out] ciphertext_len Pointer to plaintext length
 * @param[out] ciphertext Pointer to plaintext output array
 *
 * @return
 * - RET_OK on success
 * - RET_FAULT on error
 * @see #serialize_msg
 */
retcode_t deserialize_msg(char *msg, const uint8_t *iv, uint32_t *ciphertext_len, char *ciphertext);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_H
