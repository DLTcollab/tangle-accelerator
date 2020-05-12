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
  /** Optional. Service token which should be provided by the paid users. */
  char service_token[SERVICE_TOKEN_LEN + 1];
  /** Required. Data identifier. This struct pointer points to the implementation of data ID struct. */
  void* data;
  /** Optional. Decryption key. This struct pointer points to the implementation of payload decryption key struct. */
  void* key;
  /** Required. Specified protocol. The protocol used in the current data. */
  mam_protocol_t protocol;
} ta_send_mam_req_t;

/** struct of send_mam_data_mam_v1_t */
typedef struct send_mam_data_mam_v1_s {
  /** Optional. MAM channel seed. Assigning with load balancer in LAN. Assinging with public requests
   * are accepted, but tangle-accelerator won't ensure the security during seed transmission. */
  tryte_t* seed;
  /** Optional. The depth of channel merkle tree. */
  int32_t ch_mss_depth;
  /** Optional. The Channel ID which tangle-accelerator starts to search available message slot. */
  tryte_t* chid;
  /** Required. The message will be append to the channel. */
  char* message;
} send_mam_data_mam_v1_t;

typedef struct send_mam_key_mam_v1_s {
  /** Optional. The pre-shared key to encrypt the message. Each psk is in length of 81 trytes. Default: NULL. */
  UT_array* psk_array;
  /** Optional. The NTRU public key to encrypt the message. Each psk is in length of 1024 trytes. Default: NULL. */
  UT_array* ntru_array;
} send_mam_key_mam_v1_t;

/**
 * Allocate memory of ta_send_mam_req_t
 *
 * @return
 * - struct of ta_send_mam_req_t on success
 * - NULL on error
 */
ta_send_mam_req_t* send_mam_req_new();

/**
 * Free memory of ta_send_mam_req_t
 *
 * @param req Data type of ta_send_mam_req_t
 */
void send_mam_req_free(ta_send_mam_req_t** req);

/**
 * Initialize a `ta_send_mam_req_t` object as a mam_v1 object.
 *
 * @return
 * - struct of ta_send_mam_req_t on success
 * - NULL on error
 */
status_t send_mam_req_v1_init(ta_send_mam_req_t* req);

/**
 * @brief Return NTRU public key at assigning index.
 *
 * @param req Data type of ta_send_mam_req_t
 * @param index Assigning index for fecthing NTRU public key.
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
 * @param req Data type of ta_send_mam_req_t
 * @param index Assigning index for fecthing Pre-Shared Key.
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
