/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SERIALIZER_SERIALIZER_H_
#define SERIALIZER_SERIALIZER_H_

#include "cJSON.h"
#include "cclient/response/responses.h"
#include "common/trinary/tryte_ascii.h"
#include "request/request.h"
#include "response/response.h"
#include "utils/char_buffer.h"
#include "utils/containers/hash/hash_array.h"
#include "utils/fill_nines.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @file serializer.h
 * @brief Serialization of data strings
 * @example test_serializer.c
 */

//
/** @name Default input fields */
/** @{ */
/** Tag with mnemonic presentation of 'Powered by Tangle-Accelerator' */
#define DEFAULT_TAG "POWEREDBYTANGLEACCELERATOR9"
/** Address with mnemonic presentation of 'Powered by Tangle-Accelerator' */
#define DEFAULT_ADDRESS                      \
  "POWEREDBYTANGLEACCELERATOR99999999999999" \
  "99999999999999999999999999999999999999999"
/** 'Powered by tangle-accelerator' in ASCII */
#define DEFAULT_MSG "ZBCDKDTCFDTCSCEAQCMDEAHDPCBDVC9DTCRAPCRCRCTC9DTCFDPCHDCDFD"
/** Default message length */
#define DEFAULT_MSG_LEN 58
/** @} */

/**
 * Initialize logger
 */
void serializer_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int serializer_logger_release();

/**
 * @brief Serialze type of ta_generate_address_res_t to JSON string
 *
 * @param[out] obj Address hash in JSON
 * @param[in] res Response data in type of ta_generate_address_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_generate_address_res_serialize(const ta_generate_address_res_t* const res, char** obj);

/**
 * @brief Serialze object `get_tips_res_t` into JSON
 *
 * @param[in] res Result `get_tips_res_t` object with tips inside
 * @param[out] obj Output tips in JSON
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_tips_res_serialize(const get_tips_res_t* const res, char** obj);

/**
 * @brief Deserialze JSON string to type of ta_send_transfer_req_t
 *
 * @param[in] obj Input values in JSON
 * @param[out] req Request data in type of ta_send_transfer_req_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_transfer_req_deserialize(const char* const obj, ta_send_transfer_req_t* req);

/**
 * @brief Serialze the response of api_send_transfer()
 *
 * @param[in] res Response data in type of transaction_array_t
 * @param[out] obj Input values in JSON
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_transfer_res_serialize(transaction_array_t* res, char** obj);

/**
 * @brief Deserialze JSON string to hash8019_array_p
 *
 * @param[in] obj Input values in JSON
 * @param[out] out_trytes trytes arrary in the request data in type of
 * hash8019_array_p
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_trytes_req_deserialize(const char* const obj, hash8019_array_p out_trytes);

/**
 * @brief Serialze hash8019_array_p to JSON string
 *
 * @param[in] trytes trytes array returned in type of hash8019_array_p
 * @param[out] obj output serialized JSON values
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_trytes_res_serialize(const hash8019_array_p trytes, char** obj);

/**
 * @brief Serialze response of api_transaction_object_single into JSON
 *
 * @param[in] res Transaction object array, but we take only the first one
 * @param[out] obj Result of serialization in JSON format.
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transaction_object_single_res_serialize(transaction_array_t* res, char** obj);

/**
 * @brief Deserialze type of ta_find_transaction_objects_req_t from JSON string
 *
 * @param[in] obj List of transaction hashes
 * @param[out] res Response data in type of ta_find_transaction_objects_req_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transaction_objects_req_deserialize(const char* const obj,
                                                     ta_find_transaction_objects_req_t* const req);

/**
 * @brief Serialze type of ta_find_transaction_objects_res_t to JSON string
 *
 * @param[out] obj List of transaction object in JSON
 * @param[in] res Response data in type of ta_find_transaction_objects_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transaction_objects_res_serialize(const transaction_array_t* const res, char** obj);

/**
 * @brief Serialze type of ta_find_transactions_by_tag_res_t to JSON string
 *
 * @param[out] obj List of transaction hash in JSON
 * @param[in] res Response data in type of ta_find_transactions_by_tag_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transactions_by_tag_res_serialize(const ta_find_transactions_by_tag_res_t* const res, char** obj);

/**
 * @brief Serialize response of mam message
 *
 * @param[out] obj message formed in JSON
 * @param[in] message Response of payload message
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t receive_mam_message_res_serialize(char* const message, char** obj);

/**
 * @brief Deserialze JSON string to type of ta_send_mam_req_t
 *
 * @param[in] obj Input values in JSON
 * @param[out] req Request data in type of ta_send_mam_req_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_req_deserialize(const char* const obj, ta_send_mam_req_t* req);

/**
 * @brief Deserialze JSON string to type of ta_send_mam_res_t
 *
 * @param[in] obj Input values in JSON
 * @param[out] res Response data in type of ta_send_mam_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_deserialize(const char* const obj, ta_send_mam_res_t* const res);

/**
 * @brief Serialze type of ta_send_mam_res_t to JSON string
 *
 * @param[out] obj send mam response object in JSON
 * @param[in] res Response data in type of ta_send_mam_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_res_serialize(const ta_send_mam_res_t* const res, char** obj);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_SERIALIZER_H_
