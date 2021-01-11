/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */
#include <stddef.h>
#include <stdint.h>
#include "common/ta_errors.h"

#include "interfaces.h"
#include "legato.h"

#define ENDPOINT_DEVICE_KEY_NAME "Device key"

/**
 * @brief Set the device key
 *
 * @param[in] key The uint8_t array of the private key
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_SET_KEY_ERROR on error
 */
status_t set_device_key(const char *key);

/**
 * @brief Get the device key
 *
 * @param[out] key The output array for the private key
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_GET_KEY_ERROR on error
 */
status_t get_device_key(uint8_t *key);

/**
 * @brief Get the device id
 *
 * @param[out] id The output array for the device id
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_GET_DEVICE_ID_ERROR on error
 */
status_t get_device_id(char *id);
