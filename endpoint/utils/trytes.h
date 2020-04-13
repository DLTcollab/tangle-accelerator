/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef TRYTE_MSG_H
#define TRYTE_MSG_H

#include <stdint.h>

/**
 * @brief Generate randomized trytes generation
 *
 * @param[in] len Length of output
 * @param[out] out The pointer to output array
 */
void gen_rand_trytes(const uint16_t len, uint8_t *out);

/**
 * @brief create unify message to send to tangle accerlator
 *
 * @param[in] tryte_msg Byte message which has converted to tryte
 * @param[in] addr IOTA next address
 * @param[out] req_body[1024] Message array
 */
void gen_trytes_message(const char *tryte_msg, const uint8_t *addr, char req_body[1024]);

#endif  // TRYTE_MSG_H
