/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_PERIODICAL_TASK_H_
#define CORE_PERIODICAL_TASK_H_

#include <errno.h>
#include "accelerator/config.h"
#include "accelerator/core/core.h"
#include "accelerator/core/mam_core.h"
#include "common/logger.h"
#include "common/ta_errors.h"
#include "pthread.h"
#include "time.h"
#include "uuid/uuid.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/periodical_task.h
 */

void* health_track(void* arg);

/**
 * @brief Broadcast transactions in transaction buffer
 *
 * Failed transactions would be stored in transaction buffer. Once tangle-accelerator retrieve the connetion with
 * Tangle, then tangle-accelerator will start to broadcast these failed transaction trytes.
 *
 * @param[in] core Pointer to Tangle-accelerator core configuration structure
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t broadcast_buffered_txn(const ta_core_t* const core);

/**
 * @brief Broadcast buffered MAM requests
 *
 * @param[in] core Pointer to Tangle-accelerator core configuration structure
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t broadcast_buffered_send_mam_request(const ta_core_t* const core);

#ifdef __cplusplus
}
#endif

#endif  // CORE_PERIODICAL_TASK_H_
