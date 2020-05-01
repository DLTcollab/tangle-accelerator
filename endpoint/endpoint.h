/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ENDPOINT_H
#define ENDPOINT_H

#define ADDR_LEN 81
#define MAX_MSG_LEN 1024

#include <stdint.h>
#include "common/ta_errors.h"

/**
 * @brief Send transaction information to tangle accelerator
 *
 * @param[in] value Amount of the IOTA currency will be sent
 * @param[in] message Message of the transaction in Trytes format
 * @param[in] message_fmt Treating message field as specified format. Can be one of `ascii` or `trytes`. Default:
 * `ascii`
 * @param[in] tag Tag of transactions into several classifications. Tag is 27-trytes characters, e.g.
 * POWEREDBYTANGLEACCELERATOR9
 * @param[in] address Address of the receiver where IOTA currency will be sent to
 * @param[in] next_address Next address to be sent inside message
 * @param[in] private_key Private key from device
 * @param[in] device_id Device id from device
 * @param[in/out] iv Initialization vector, must be read/write. The length of iv must be AES_IV_SIZE @see #ta_cipher_ctx
 *
 * @return #status_t
 */
status_t send_transaction_information(const int value, const char* message, const char* message_fmt, const char* tag,
                                      const char* address, const char* next_address, const uint8_t* private_key,
                                      const char* device_id, uint8_t* iv);

#endif  // ENDPOINT_H