/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SERIALIZER_SLZ_HELPER_H_
#define SERIALIZER_SLZ_HELPER_H_

#include "accelerator/core/request/request.h"
#include "accelerator/core/response/response.h"
#include "cJSON.h"
#include "common/logger.h"
#include "common/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/serializer/ser_helper.h
 * @brief Helper functions for serialization
 */

logger_id_t ser_logger_id;

/**
 * @brief Examine whether the given string is a list of trytes in 'len' long
 *
 * @param[in] trytes The string to be examined.
 * @param[in] len The length we to examine.
 *
 * @return
 * - true on A trytes list in given length
 * - false on Not a trytes list in given length
 */
bool valid_tryte(char* trytes, int len);

/**
 * @brief Convert cJSON array object to string array in utarray
 *
 * @param[in] obj Input cJSON string array
 * @param[in] obj_name The name of this element in JSON
 * @param[out] ut Output string array in utarray datatype
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_string_array_to_string_utarray(cJSON const* const obj, char const* const obj_name, UT_array* const ut);

/**
 * @brief Convert hash243_stack_t to cJSON array element
 *
 * @param[in] stack hash243_stack_t object
 * @param[out] json_root Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_hash243_stack_to_json_array(hash243_stack_t stack, cJSON* json_root);

/**
 * @brief Convert cJSON array object to hash243_queue_t object
 *
 * @param[in] obj Input cJSON string array
 * @param[in] obj_name The name of this element in JSON
 * @param[out] queue Output hash243_queue_t object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_array_to_hash243_queue(cJSON const* const obj, char const* const obj_name, hash243_queue_t* queue);

/**
 * @brief Convert hash243_queue_t to cJSON array element
 *
 * @param[in] queue hash243_queue_t object
 * @param[out] json_root Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_hash243_queue_to_json_array(hash243_queue_t queue, cJSON* const json_root);

/**
 * @brief Convert cJSON array object to hash8019_array_p object
 *
 * @param[in] obj Input cJSON string array
 * @param[in] obj_name The name of this element in JSON
 * @param[out] array Output hash8019_array_p object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_array_to_hash8019_array(cJSON const* const obj, char const* const obj_name, hash8019_array_p array);

/**
 * @brief Convert hash8019_array_p to cJSON array element
 *
 * @param[in] array hash8019_array_p object
 * @param[in] obj_name The name of this element in JSON
 * @param[out] json_root Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_hash8019_array_to_json_array(hash8019_array_p array, char const* const obj_name, cJSON* const json_root);

/**
 * @brief Convert iota_transaction_t object to cJSON object
 *
 * @param[in] txn Input iota_transaction_t object
 * @param[out] txn_json Output cJSON string object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_iota_transaction_to_json_object(iota_transaction_t const* const txn, cJSON** txn_json);

/**
 * @brief Convert transaction_array_t to cJSON array element
 *
 * @param[in] txn_array transaction_array_t object
 * @param[out] json_root Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_transaction_array_to_json_array(const transaction_array_t* const txn_array, cJSON* json_root);

/**
 * @brief Convert bundle_transactions_t to cJSON array element
 *
 * @param[in] bundle bundle_transactions_t object
 * @param[in] obj_name The name of this element in JSON
 * @param[out] json_root Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_bundle_transaction_to_json_array(const bundle_transactions_t* const bundle, char const* const obj_name,
                                             cJSON* json_root);

/**
 * @brief Get the string value from JSON
 *
 * @param[in] json_obj Input cJSON object
 * @param[in] obj_name The name of this element in JSON
 * @param[in,out] text Output string value
 * @param[in] size The length of string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_get_string(cJSON const* const json_obj, char const* const obj_name, char* const text,
                            const size_t size);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_SLZ_HELPER_H_
