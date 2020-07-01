/*
 * Copyright (C) 2019-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_MAM_CORE_H_
#define CORE_MAM_CORE_H_

#include "accelerator/core/core.h"
#include "common/macros.h"
#include "common/trinary/flex_trit.h"
#include "common/trinary/tryte_ascii.h"
#include "mam/api/api.h"
#include "mam/mam/mam_channel_t_set.h"
#include "utarray.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/mam_core.h
 */

/**
 * Initialize logger for 'mam_core'
 */
void ta_mam_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int ta_mam_logger_release();

typedef enum mam_send_operation_e { ANNOUNCE_CHID, SEND_MESSAGE } mam_send_operation_t;

typedef struct mam_encrypt_key_s {
  mam_psk_t_set_t psks;
  mam_ntru_pk_t_set_t ntru_pks;
  mam_ntru_sk_t_set_t ntru_sks;
} mam_encrypt_key_t;

/**
 * @brief Send a MAM message.
 *
 * @param[in] info Tangle-accelerator configuration variables
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA node service
 * @param[in] req Request in 'ta_send_mam_req_t' datatype
 * @param[out] res Result in 'ta_send_mam_res_t' datatype
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_send_mam_message(const ta_config_t* const info, const iota_config_t* const iconf,
                             const iota_client_service_t* const service, ta_send_mam_req_t const* const req,
                             ta_send_mam_res_t* const res);

/**
 * @brief Receive MAM messages.
 *
 * @param[in] iconf IOTA API parameter configurations
 * @param[in] service IOTA node service
 * @param[in] req Request in 'ta_recv_mam_req_t' datatype
 * @param[out] res Result in 'ta_recv_mam_res_t' datatype
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_recv_mam_message(const iota_config_t* const iconf, const iota_client_service_t* const service,
                             ta_recv_mam_req_t* const req, ta_recv_mam_res_t* const res);

#ifdef __cplusplus
}
#endif

#endif  // CORE_MAM_CORE_H_
