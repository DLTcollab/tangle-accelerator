/*
 * Copyright (C) 2019 BiiLabs Co., Ltd. and Contributors
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

typedef enum mam_protocol_e { MAM_V1 } mam_protocol_t;

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

typedef struct data_id_mam_v1_s {
  /** ascii string bundle hash */
  char* bundle_hash;
  /** ascii string channel id */
  char* chid;
  /** ascii string endpoint id */
  char* epid;
  /** channel ordinal which is the number of channel we generated */
  char* msg_id;
} data_id_mam_v1_t;

typedef struct key_mam_v1_s {
  /** The pre-shared key to encrypt the message. Length: 81 trytes. Default: NULL. */
  tryte_t* psk;
  /** The NTRU public key to encrypt the message. Length: 1024 trytes. Default: NULL. */
  tryte_t* ntru_pk;
} key_mam_v1_t;

status_t set_mam_v1_data_id(ta_recv_mam_req_t* req, char* bundle_hash, char* chid, char* epid, char* msg_id);
status_t set_mam_v1_key(ta_recv_mam_req_t* req, tryte_t* psk, tryte_t* ntru);

#ifdef __cplusplus
}
#endif

#endif  // REQUEST_TA_RECV_MAM_H_
