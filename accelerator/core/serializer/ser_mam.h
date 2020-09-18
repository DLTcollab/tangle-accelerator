/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef SERIALIZER_SLZ_MAM_H_
#define SERIALIZER_SLZ_MAM_H_

#include "accelerator/core/request/request.h"
#include "accelerator/core/response/response.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/serializer/ser_mam.h
 * @brief Functions for MAM request/response serialization
 */

/**
 * @brief Deserialize JSON string to type of ta_send_mam_req_t
 *
 * @param[in] obj Input values in JSON
 * @param[out] req Request data in type of ta_send_mam_req_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_message_req_deserialize(const char* const obj, ta_send_mam_req_t* req);

/**
 * @brief Deserialize JSON string to type of ta_send_mam_res_t
 *
 * @param[in] obj Input values in JSON
 * @param[out] res Response data in type of ta_send_mam_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_message_res_deserialize(const char* const obj, ta_send_mam_res_t* const res);

/**
 * @brief Serialize type of ta_send_mam_res_t to JSON string
 *
 * @param[out] obj send mam response object in JSON
 * @param[in] res Response data in type of ta_send_mam_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t send_mam_message_res_serialize(const ta_send_mam_res_t* const res, char const* const uuid, char** obj);

/**
 * @brief Deserialize request of recv_mam_message
 *
 * @param[in] obj message formed in JSON
 * @param[out] req ta_recv_mam_req_t object
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t recv_mam_message_req_deserialize(const char* const obj, ta_recv_mam_req_t* const req);

/**
 * @brief Deserialize JSON string to type of ta_recv_mam_res_t
 *
 * @param[in] obj Input values in JSON
 * @param[out] res Response in datatype ta_recv_mam_res_t
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t recv_mam_message_res_deserialize(const char* const obj, ta_recv_mam_res_t* const res);

/**
 * @brief Serialize response of mam message
 *
 * @param[in] res Response of payload message in 'ta_recv_mam_res_t' datatype
 * @param[out] obj Message array formed in JSON
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t recv_mam_message_res_serialize(ta_recv_mam_res_t* const res, char** obj);

#ifdef __cplusplus
}
#endif

#endif  // SERIALIZER_SLZ_MAM_H_
