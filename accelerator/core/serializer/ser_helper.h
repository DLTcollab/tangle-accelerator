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

logger_id_t ser_logger_id;

bool valid_tryte(char* trytes, int len);

/**
 * @brief Convert string array in utarray to cJSON array element
 *
 * @param ut[in] String array in utarray datatype
 * @param name[in] The name of this element in JSON
 * @param json_root[out] Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_string_utarray_to_json_array(UT_array const* const ut, char const* const obj_name, cJSON* const json_root);

/**
 * @brief Convert cJSON array object to string array in utarray
 *
 * @param json_root[in] Input cJSON string array
 * @param name[in] The name of this element in JSON
 * @param ut[out] Output string array in utarray datatype
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_string_array_to_string_utarray(cJSON const* const obj, char const* const obj_name, UT_array* const ut);

/**
 * @brief Convert hash243_stack_t to cJSON array element
 *
 * @param stack[in] hash243_stack_t object
 * @param json_root[out] Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_hash243_stack_to_json_array(hash243_stack_t stack, cJSON* json_root);

/**
 * @brief Convert cJSON array object to hash243_queue_t object
 *
 * @param json_root[in] Input cJSON string array
 * @param name[in] The name of this element in JSON
 * @param ut[out] Output hash243_queue_t object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_array_to_hash243_queue(cJSON const* const obj, char const* const obj_name, hash243_queue_t* queue);

/**
 * @brief Convert hash243_queue_t to cJSON array element
 *
 * @param queue[in] hash243_queue_t object
 * @param json_root[out] Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_hash243_queue_to_json_array(hash243_queue_t queue, cJSON* const json_root);

/**
 * @brief Convert cJSON array object to hash8019_array_p object
 *
 * @param json_root[in] Input cJSON string array
 * @param name[in] The name of this element in JSON
 * @param array[out] Output hash8019_array_p object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_json_array_to_hash8019_array(cJSON const* const obj, char const* const obj_name, hash8019_array_p array);

/**
 * @brief Convert hash8019_array_p to cJSON array element
 *
 * @param array[in] hash8019_array_p object
 * @param name[in] The name of this element in JSON
 * @param json_root[out] Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_hash8019_array_to_json_array(hash8019_array_p array, char const* const obj_name, cJSON* const json_root);

/**
 * @brief Convert iota_transaction_t object to cJSON object
 *
 * @param txn[in] Input iota_transaction_t object
 * @param name[in] The name of this element in JSON
 * @param txn_json[out] Output cJSON string object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_iota_transaction_to_json_object(iota_transaction_t const* const txn, cJSON** txn_json);

/**
 * @brief Convert transaction_array_t to cJSON array element
 *
 * @param txn_array[in] transaction_array_t object
 * @param name[in] The name of this element in JSON
 * @param json_root[out] Output cJSON object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_transaction_array_to_json_array(const transaction_array_t* const txn_array, cJSON* json_root);

/**
 * @brief Convert bundle_transactions_t to cJSON array element
 *
 * @param bundle[in] bundle_transactions_t object
 * @param name[in] The name of this element in JSON
 * @param json_root[out] Output cJSON object
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
 * @param json_obj[in] Input cJSON object
 * @param obj_name[in] The name of this element in JSON
 * @param text[in/out] Output string value
 * @param size[in] The length of string
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
