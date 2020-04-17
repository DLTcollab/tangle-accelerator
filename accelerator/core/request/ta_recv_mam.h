/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef REQUEST_TA_RECV_MAM_H_
#define REQUEST_TA_RECV_MAM_H_

#include <stdlib.h>
#include <string.h>
#include "common/macros.h"
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/request/ta_recv_mam.h
 */

/** struct of ta_recv_mam_req_t */
typedef struct recv_mam_req_s {
  /** Required. Data identifier. This struct pointer points to the implementation of data ID struct. */
  void* data_id;
  /** Required. Decryption key. This struct pointer points to the implementation of payload decryption key struct. */
  void* key;
  /** Required. Specified protocol. The protocol used in the current data. */
  mam_protocol_t protocol;
} ta_recv_mam_req_t;

/**
 * Allocate memory of ta_recv_mam_req_t
 *
 * @return
 * - struct of ta_recv_mam_req_t on success
 * - NULL on error
 */
ta_recv_mam_req_t* recv_mam_req_new();

/**
 * Free memory of ta_recv_mam_req_t
 *
 * @param req Data type of ta_recv_mam_req_t
 */
void recv_mam_req_free(ta_recv_mam_req_t** req);

// TODO Implement interface of MAMv1.1

typedef struct recv_mam_data_id_mam_v1_s {
  /** bundle hash in trytes */
  tryte_t* bundle_hash;
  /** channel id in trytes */
  tryte_t* chid;
  /** endpoint id in trytes */
  tryte_t* epid;
  /** message id in trytes */
  tryte_t* msg_id;
} recv_mam_data_id_mam_v1_t;

typedef struct recv_mam_key_mam_v1_s {
  /** Message encryption key. This field could be Pre-Shared Key (81 trytes) or NTRU public key (1024 trytes). Default:
   * NULL. */
  tryte_t* enc_key;
} recv_mam_key_mam_v1_t;

/**
 * Set the data ID for MAMv1
 *
 * @param[in] req Response data in type of ta_recv_mam_req_t object
 * @param[in] bundle_hash Bundle hash of the message
 * @param[in] chid Channel ID of the messages
 * @param[in] epid Endpoint ID of the messages
 * @param[in] msg_id Message ID of the message
 *
 * @return
 * - struct of ta_recv_mam_req_t on success
 * - NULL on error
 */
status_t recv_mam_set_mam_v1_data_id(ta_recv_mam_req_t* req, char* bundle_hash, char* chid, char* epid, char* msg_id);

/**
 * Set the key for MAMv1
 *
 * @param[in] req Response data in type of ta_recv_mam_req_t object
 * @param[in] psk Pre-Shared Key to decrypt message
 * @param[in] ntru NTRU public key to decrypt message
 *
 * @return
 * - struct of ta_recv_mam_req_t on success
 * - NULL on error
 */
status_t recv_mam_set_mam_v1_key(ta_recv_mam_req_t* req, tryte_t* psk, tryte_t* ntru);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_RECV_MAM_H_
