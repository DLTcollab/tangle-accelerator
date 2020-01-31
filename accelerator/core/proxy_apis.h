/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CORE_PROXY_APIS_H_
#define CORE_PROXY_APIS_H_

#include "cclient/api/core/core_api.h"
#include "cclient/request/requests.h"
#include "cclient/response/responses.h"
#include "serializer/serializer.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/core/proxy_apis.h
 * @brief Implement Proxy APIs
 *
 * tangle-accelerator provides major IRI Proxy APIs wrapper.
 * The arguments and return strings are all in json format. There can be
 * different server or protocol integration with these APIs.
 */

/**
 * Initialize logger
 */
void proxy_apis_logger_init();

/**
 * Release logger
 *
 * @return
 * - zero on success
 * - EXIT_FAILURE on error
 */
int proxy_apis_logger_release();

/**
 * Initialize lock
 *
 * @return
 * - zero on success
 * - SC_CONF_LOCK_INIT on error
 */
status_t proxy_apis_lock_init();

/**
 * Destroy lock
 *
 * @return
 * - zero on success
 * - SC_CONF_LOCK_DESTROY on error
 */
status_t proxy_apis_lock_destroy();

/**
 * @brief Proxy API of IOTA core functionalities
 *
 * @param[in] service IRI node end point service
 * @param[in] obj IOTA core APIs request body
 * @param[out] json_result Result of IOTA core APIs
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t proxy_api_wrapper(const ta_config_t* const iconf, const iota_client_service_t* const service,
                           const char* const obj, char** json_result);

#ifdef __cplusplus
}
#endif

#endif  // CORE_PROXY_APIS_H_
