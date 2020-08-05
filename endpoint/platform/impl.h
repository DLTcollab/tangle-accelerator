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

/**
 * @brief Initialize the secure storage platformService
 *
 * @return
 * - SC_OK on success
 */
status_t sec_init(void);

/**
 * @brief Write item to secure storage
 *
 * @param[in] name Name of the secure storage item
 * @param[in] buf Buffer containing the data to store
 * @param[in] buf_size Sizeof the item
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_SEC_FAULT on error
 * - SC_ENDPOINT_SEC_UNAVAILABLE the secure storage is currently unavailable
 */
status_t sec_write(const char *name, const uint8_t *buf, size_t buf_size);

/**
 * @brief Read item from secure storage
 *
 * @param[in] name Name of the secure storage item
 * @param[out] buf Buffer to store the data in
 * @param[in,out] buf_size Size of the data
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_SEC_ITEM_NOT_FOUND the item cannot found
 * - SC_ENDPOINT_SEC_UNAVAILABLE the secure storage is currently unavailable
 * - SC_ENDPOINT_SEC_FAULT on error
 */
status_t sec_read(const char *name, uint8_t *buf, size_t *buf_size);

/**
 * @brief Delete item from secure storage
 *
 * @param[in] name Name of the secure storage item
 * @return
 * - SC_OK on success
 * - SC_ENDPOINT_SEC_ITEM_NOT_FOUND the item cannot found
 * - SC_ENDPOINT_SEC_UNAVAILABLE the secure storage is currently unavailable
 * - SC_ENDPOINT_SEC_FAULT on error
 */
status_t sec_delete(const char *name);
