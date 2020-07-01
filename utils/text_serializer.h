/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef UTILS_TEXT_SERIALIZER_H
#define UTILS_TEXT_SERIALIZER_H

#include <stddef.h>
#include <stdint.h>
#include "common/ta_errors.h"
#include "utils/cipher.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file utils/text_serializer.h
 */

/**
 * @brief Serialize ciphertext and initialize vector together, the out put message
 * format show as below:
 * - initialize vector(16 bytes)
 * - timestamp (20 bytes)
 * - hmac (32 bytes)
 * - ciphertext length(10 bytes)
 * - ciphertext(other bytes)
 *
 * @param[in] ctx The cipher context to be serialized
 * @param[out] out_msg Pointer to output message
 * @param[out] out_msg_len Pointer to length of serialized message
 *
 * @return
 * - SC_OK on success
 * - SC_UTILS_TEXT_SERIALIZE on error
 */
status_t serialize_msg(const ta_cipher_ctx *ctx, char *out_msg, size_t *out_msg_len);

/**
 * @brief Deserialize message from serialize_msg
 *
 * @param[in] msg Pointer to serialize message
 * @param[in,out] ctx The cipher context to be deserialized
 *
 * @return
 * - SC_OK on success
 * - SC_UTILS_TEXT_DESERIALIZE on error
 * @see #serialize_msg
 */
status_t deserialize_msg(const char *msg, ta_cipher_ctx *ctx);
#ifdef __cplusplus
}
#endif
#endif  // UTILS_TEXT_SERIALIZER_H
