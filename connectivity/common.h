/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef CONN_COMMON_H
#define CONN_COMMON_H
#include "common/ta_errors.h"

#ifdef __cplusplus
extern "C" {
#endif

#define STR_HTTP_NOT_FOUND "{\"message\": \"Request not found\"}"
#define STR_HTTP_BAD_REQUEST "{\"message\": \"Invalid request header\"}"
#define STR_HTTP_BAD_REQUEST_MESSAGE_OVERSIZE "{\"message\": \"In body 'message', requested message is too long.\"}"
#define STR_HTTP_INTERNAL_SERVICE_ERROR "{\"message\": \"Internal service error\"}"
#define STR_HTTP_REQUEST_SIZE_EXCEED "{\"message\": \"Request size exceed\"}"

/**
 * @brief Match path with given regular expression rule
 * @param[in] path Path
 * @param[in] regex_rule Regex rule
 *
 * @return
 * - SC_HTTP_NULL if regular expression is NULL
 * - SC_HTTP_INVALID_REGEX if failed to compile regular expression
 * - SC_HTTP_URL_NOT_MATCH if regular expression does not match the given path
 * - SC_OK on success
 */
status_t api_path_matcher(char const *const path, char *const regex_rule);

/**
 * @brief Set http reponse content for given status_t code from functions
 *
 * @param[in] ret Status code returned from apis
 * @param[out] json_result Response content in json format
 *
 * @return HTTP status code
 */
status_t set_response_content(status_t ret, char **json_result);
void conn_logger_init();
int conn_logger_release();

#ifdef __cplusplus
}
#endif

#endif  // CONN_COMMON_H
