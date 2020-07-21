/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef RESPONSE_TA_RECV_MAM_H_
#define RESPONSE_TA_RECV_MAM_H_

#include "common/macros.h"
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/response/ta_recv_mam.h
 */

/** struct of ta_recv_mam_res_t */
typedef struct recv_mam_res_s {
  UT_array* payload_array;         /**< An array of MAM messages */
  char chid1[NUM_TRYTES_HASH + 1]; /**< The Channel ID of next Channel */
} ta_recv_mam_res_t;

/**
 * @brief Allocate memory of ta_recv_mam_res_t
 *
 * @return
 * - The pointer of the ta_recv_mam_res_t struct
 * - NULL on error
 */
ta_recv_mam_res_t* recv_mam_res_new();

/**
 * @brief Free memory of ta_recv_mam_res_t
 *
 * @param[in] res Pointer of pointer of ta_recv_mam_res_t object
 */
void recv_mam_res_free(ta_recv_mam_res_t** res);

#ifdef __cplusplus
}
#endif

#endif  // RESPONSE_TA_RECV_MAM_H_
