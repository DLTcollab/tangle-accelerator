/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef REQUEST_TA_SEND_MAM_H_
#define REQUEST_TA_SEND_MAM_H_

#include <stdlib.h>
#include <string.h>
#include "common/macros.h"
#include "common/ta_errors.h"
#include "common/trinary/tryte.h"
#include "utarray.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/request/ta_send_mam.h
 */

#define SERVICE_TOKEN_LEN 64

/** struct of ta_send_mam_req_t */
typedef struct send_mam_req_s {
  /*@{*/
  char service_token[SERVICE_TOKEN_LEN + 1]; /**< Optional. Service token which should be provided by the paid users. */
  void* data; /**< Required. Data identifier. This struct pointer points to the implementation of data ID struct. */
  void* key;  /**< Optional. Decryption key. This struct pointer points to the implementation of payload decryption key
                 struct. */
  mam_protocol_t protocol; /**< Required. Specified protocol. The protocol used in the current data. */
  /*@}*/
} ta_send_mam_req_t;

/** struct of send_mam_data_mam_v1_t */
typedef struct send_mam_data_mam_v1_s {
  tryte_t* seed; /**< Optional. MAM channel seed. Assigning with load balancer in LAN. Assinging with public requests
                  * are accepted, but tangle-accelerator won't ensure the security during seed transmission. */
  int32_t ch_mss_depth; /**< Optional. The depth of channel merkle tree. */
  tryte_t* chid; /**< Optional. The Channel ID which tangle-accelerator starts to search available message slot. */
  char* message; /**< Required. The message will be append to the channel. */
} send_mam_data_mam_v1_t;

/** struct of send_mam_key_mam_v1_t */
typedef struct send_mam_key_mam_v1_s {
  UT_array* psk_array;  /**< Optional. The pre-shared key to encrypt the message. Each psk is in length of 81 trytes.
                           Default: NULL. */
  UT_array* ntru_array; /**< Optional. The NTRU public key to encrypt the message. Each psk is in length of 1024 trytes.
                           Default: NULL. */
} send_mam_key_mam_v1_t;

/**
 * @brief Allocate memory of ta_send_mam_req_t
 *
 * @return
 * - struct of ta_send_mam_req_t on success
 * - NULL on error
 */
ta_send_mam_req_t* send_mam_req_new();

/**
 * @brief Free memory of ta_send_mam_req_t
 *
 * @param[in] req Data type of ta_send_mam_req_t
 */
void send_mam_req_free(ta_send_mam_req_t** req);

/**
 * @brief Initialize a `ta_send_mam_req_t` object as a mam_v1 object.
 *
 * @param[out] req 'ta_send_mam_req_t' object to be initialized
 *
 * @return
 * - struct of ta_send_mam_req_t on success
 * - NULL on error
 */
status_t send_mam_req_v1_init(ta_send_mam_req_t* req);

/**
 * @brief Return NTRU public key at assigning index.
 *
 * @param[in] req 'ta_send_mam_req_t' object
 * @param[in] index Assigning index for fecthing NTRU public key.
 *
 * @return
 * - NTRU public key in const char*
 */
static inline char const* mamv1_ntru_key_at(ta_send_mam_req_t* req, unsigned int index) {
  send_mam_key_mam_v1_t* key = (send_mam_key_mam_v1_t*)req->key;
  return *(char const**)utarray_eltptr(key->ntru_array, index);
}

/**
 * @brief Return Pre-Shared Key at assigning index.
 *
 * @param[in] req Data type of ta_send_mam_req_t
 * @param[in] index Assigning index for fecthing Pre-Shared Key.
 *
 * @return
 * - Pre-Shared Key in const char*
 */
static inline char const* mamv1_psk_key_at(ta_send_mam_req_t* req, unsigned int index) {
  send_mam_key_mam_v1_t* key = (send_mam_key_mam_v1_t*)req->key;
  return *(char const**)utarray_eltptr(key->psk_array, index);
}

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_SEND_MAM_H_
