/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SERIALIZER_SERIALIZER_H_
#define SERIALIZER_SERIALIZER_H_

#include "accelerator/config.h"
#include "accelerator/core/request/request.h"
#include "accelerator/core/response/response.h"
#include "cJSON.h"
#include "common/trinary/tryte_ascii.h"
#include "ser_mam.h"
#include "utils/fill_nines.h"

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @file accelerator/core/serializer/serializer.h
 * @brief Serialization of data strings
 * @example unit-test/test_serializer.c
 */

/** @name Default input fields */
/** @{ */
/**
 * Tag with mnemonic presentation of 'Powered by Tangle-Accelerator'
 */
#define DEFAULT_TAG "POWEREDBYTANGLEACCELERATOR9"
/**
 * Address with mnemonic presentation of 'Powered by Tangle-Accelerator'
 */
#define DEFAULT_ADDRESS "POWEREDBYTANGLEACCELERATOR9999999999999999999999999999999999999999999999999999999"
/**
 * 'Powered by tangle-accelerator' in ASCII
 */
#define DEFAULT_MSG "ZBCDKDTCFDTCSCEAQCMDEAHDPCBDVC9DTCRAPCRCRCTC9DTCFDPCHDCDFD"
/**
 * Default message length
 */
#define DEFAULT_MSG_LEN 58
/** @} */

/**
 * @brief Serialize tangle accelerator info into JSON
 *
 * @param[out] obj Tangle-accelerator info in JSON
 * @param[in] ta_config Tangle-accelerator configuration variables
 * @param[in] tangle IOTA configuration variables
 * @param[in] cache Redis configuration variables
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_get_info_serialize(char** obj, ta_config_t* const ta_config, iota_config_t* const tangle,
                               ta_cache_t* const cache);

#ifdef DB_ENABLE
/**
 * @brief Serialize identity info into JSON
 *
 * @param[out] obj db identity info in JSON
 * @param[in] id_obj pointer to db_identity_t;
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t db_identity_serialize(char** obj, db_identity_t* id_obj);
#endif

/**
 * @brief Serialize the response of api_insert_identity()
 *
 * @param[in] hash Response transaction hash
 * @param[in] uuid_string Response uuid string
 * @param[out] obj Input values in JSON
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_insert_identity_res_serialize(const char* hash, const char* uuid_string, char** obj);

/**
 * @brief Deserialize JSON string to type of ta_send_transfer_req_t
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
 * @brief Serialize the response of api_send_transfer()
 *
 * @param[in] res Response data in type of ta_send_transfer_res_t
 * @param[out] obj Input values in JSON
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_transfer_res_serialize(ta_send_transfer_res_t* res, char** obj);

/**
 * @brief Deserialize JSON string to hash8019_array_p
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
 * @brief Serialize hash8019_array_p to JSON string
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
 * @brief Serialize response of api_transaction_object_single into JSON
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
 * @brief Deserialize type of ta_find_transaction_objects_req_t from JSON string
 *
 * @param[in] obj List of transaction hashes
 * @param[out] req Response data in type of ta_find_transaction_objects_req_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transaction_objects_req_deserialize(const char* const obj,
                                                     ta_find_transaction_objects_req_t* const req);

/**
 * @brief Serialize type of ta_find_transaction_objects_res_t to JSON string
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
 * @brief Serialize type of ta_find_transactions_by_tag_res_t to JSON string
 *
 * @param[out] obj List of transaction hash in JSON
 * @param[in] res Response data in type of ta_find_transactions_by_tag_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_find_transactions_by_tag_res_serialize(const ta_find_transactions_by_tag_res_t* const res, char** obj);

#ifdef MQTT_ENABLE
/**
 * @brief Deserialize device ID from MQTT JSON request.
 *
 * @param[in] obj Input request in JSON with device ID
 * @param[out] device_id Device ID in string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t mqtt_device_id_deserialize(const char* const obj, char* device_id);

/**
 * @brief Deserialize tag in string from MQTT JSON request.
 *
 * @param[in] obj Input request in JSON with tag field
 * @param[out] tag Tag in string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t mqtt_tag_req_deserialize(const char* const obj, char* tag);

/**
 * @brief Deserialize transaction hash in string from MQTT JSON request.
 *
 * @param[in] obj Input request in JSON with hash field
 * @param[out] hash Transaction hash in string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t mqtt_transaction_hash_req_deserialize(const char* const obj, char* hash);
#endif

/**
 * @brief Deserialize proxy api command.
 *
 * @param[in] obj Input request in JSON with hash field
 * @param[out] command Proxy API command name in string
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t proxy_apis_command_req_deserialize(const char* const obj, char* command);

/**
 * @brief Deserialize latestMilestone and latestSolidSubtangleMilestone from IOTA core API getNodeInfo
 *
 * @param[in] obj getNodeInfo response in JSON.
 * @param[out] latestMilestone Index of latestMilestone
 * @param[out] latestSolidSubtangleMilestone Index of latestSolidSubtangleMilestone
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t get_node_status_milestone_deserialize(char const* const obj, int* const latestMilestone,
                                               int* const latestSolidSubtangleMilestone);

/**
 * @brief Serialize the response of IOTA full node connection status.
 *
 * @param[in] status Reponse status code
 * @param[out] obj Serialized API response
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t get_node_status_res_serialize(const status_t status, char** obj);

status_t fetch_buffered_request_status_req_deserialize(char* obj, char* uuid);

/**
 * @brief Serialize the response of `fetch_buffered_request_status()`.
 *
 * @param[in] res ta_fetch_buffered_request_status_res_t object
 * @param[out] obj Serialized API response
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t fetch_buffered_request_status_res_serialize(const ta_fetch_buffered_request_status_res_t* const res,
                                                     char** obj);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_SERIALIZER_H_
