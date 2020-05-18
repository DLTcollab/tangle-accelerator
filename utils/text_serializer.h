/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef TEXT_SERIALIZER_H
#define TEXT_SERIALIZER_H

#include <stddef.h>
#include <stdint.h>
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Serialize ciphertext and initialize vector together, the out put message
 * format show as below:
 * - initialize vector(16 bytes)
 * - timestamp (20 bytes)
 * - hmac (32 bytes)
 * - ciphertext length(10 bytes)
 * - ciphertext(other bytest)
 *
 * @param[in] iv Pointer to initialize vector
 * @param[in] ciphertext_len Length of ciphertext
 * @param[in] ciphertext Ciphertext to be serialized
 * @param[in] timestamp Timestamp to be serialized
 * @param[in] hmac Hash mac array to be serialized
 * @param[out] out_msg Pointer to output message
 * @param[out] out_msg_len Pointer to length of serialized message
 *
 * @return
 * - SC_OK on success
 * - SC_UTILS_TEXT_SERIALIZE on error
 */
status_t serialize_msg(const uint8_t *iv, uint32_t ciphertext_len, const char *ciphertext, const uint64_t timestamp,
                       const uint8_t *hmac, char *out_msg, size_t *out_msg_len);

/**
 * @brief Deserialize message from serialize_msg
 *
 * @param[in] msg Pointer to serialize message
 * @param[in] iv Pointer to initialize vector
 * @param[out] ciphertext_len Pointer to plaintext length
 * @param[out] ciphertext Pointer to plaintext output array
 * @param[out] timestamp Pointer to timestamp
 * @param[out] hmac Pointer to hash mac output array
 *
 * @return
 * - SC_OK on success
 * - SC_UTILS_TEXT_DESERIALIZE on error
 * @see #serialize_msg
 */
status_t deserialize_msg(char *msg, const uint8_t *iv, size_t *ciphertext_len, char *ciphertext, uint64_t *timestamp,
                         uint8_t *hmac);
#ifdef __cplusplus
}
#endif
#endif  // TEXT_SERIALIZER_H
