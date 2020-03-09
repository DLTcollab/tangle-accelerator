/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef COMMON_TA_ERRORS_H_
#define COMMON_TA_ERRORS_H_

#include <stdbool.h>
#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file common/ta_errors.h
 * @brief Error Code of tangle-accelerator
 *
 * bit division:
 * 0 - 5  actual error code
 * 6 - 7  serverity
 * 8 - 15 error module
 *
 * *--------*--------*
 * |MMMMMMMM|SSCCCCCC|
 * *--------*--------*
 */

/** @name serverity code */
/** @{ */
#define SC_SEVERITY_MASK 0x00C0
#define SC_SEVERITY_SHIFT 6

#define SC_SEVERITY_MINOR (0x0 << SC_SEVERITY_SHIFT)
#define SC_SEVERITY_MODERATE (0x01 << SC_SEVERITY_SHIFT)
#define SC_SEVERITY_MAJOR (0x02 << SC_SEVERITY_SHIFT)
#define SC_SEVERITY_FATAL (0x03 << SC_SEVERITY_SHIFT)
/** @} */

/** @name module code */
/** @{ */
#define SC_MODULE_MASK 0xFF00
#define SC_MODULE_SHIFT 8

#define SC_MODULE_TA (0x01 << SC_MODULE_SHIFT)
#define SC_MODULE_CCLIENT (0x02 << SC_MODULE_SHIFT)
#define SC_MODULE_SERIALIZER (0x03 << SC_MODULE_SHIFT)
#define SC_MODULE_CACHE (0x04 << SC_MODULE_SHIFT)
#define SC_MODULE_MAM (0x05 << SC_MODULE_SHIFT)
#define SC_MODULE_RES (0x06 << SC_MODULE_SHIFT)
#define SC_MODULE_CONF (0x07 << SC_MODULE_SHIFT)
#define SC_MODULE_UTILS (0x08 << SC_MODULE_SHIFT)
#define SC_MODULE_HTTP (0x09 << SC_MODULE_SHIFT)
#define SC_MODULE_MQTT (0x0A << SC_MODULE_SHIFT)
#define SC_MODULE_STORAGE (0x0B << SC_MODULE_SHIFT)
#define SC_MODULE_CORE (0x0C << SC_MODULE_SHIFT)
#define SC_MODULE_ENDPOINT (0x0D << SC_MODULE_SHIFT)
/** @} */

/** @name serverity code */
/** @{ */
#define SC_ERROR_MASK 0x003F
/** @} */

/* status code */
typedef enum {
  SC_OK = 0, /**< No error */

  SC_HTTP_OK = 200,          /**< HTTP response OK */
  SC_HTTP_BAD_REQUEST = 400, /**< HTTP response, error when parsing request */
  SC_HTTP_NOT_FOUND = 404,   /**< HTTP request not found */
  SC_HTTP_INTERNAL_SERVICE_ERROR = 500,
  /**< HTTP response, other errors in TA */

  SC_TA_OOM = 0x01 | SC_MODULE_TA | SC_SEVERITY_FATAL,
  /**< Failed to create TA object */
  SC_TA_NULL = 0x02 | SC_MODULE_TA | SC_SEVERITY_FATAL,
  /**< NULL TA objects */
  SC_TA_WRONG_REQUEST_OBJ = 0x03 | SC_MODULE_TA | SC_SEVERITY_FATAL,
  /**< wrong TA request object */
  SC_TA_LOGGER_INIT_FAIL = 0x04 | SC_MODULE_TA | SC_SEVERITY_MAJOR,
  /**< Failed to init TA logger */

  // CClient module
  SC_CCLIENT_OOM = 0x01 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Failed to create cclient object */
  SC_CCLIENT_NOT_FOUND = 0x02 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Empty result from cclient */
  SC_CCLIENT_FAILED_RESPONSE = 0x03 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Error in cclient response */
  SC_CCLIENT_INVALID_FLEX_TRITS = 0x04 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< Invalid flex trits */
  SC_CCLIENT_HASH = 0x05 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< hash container operation error */
  SC_CCLIENT_JSON_KEY = 0x06 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< JSON key not found */
  SC_CCLIENT_JSON_PARSE = 0x07 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< json parsing error, might the wrong format */
  SC_CCLIENT_FLEX_TRITS = 0x09 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< Flex trits converting error */
  SC_CCLIENT_JSON_CREATE = 0x0A | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< json create object error, might OOM. */

  // Serializer module
  SC_SERIALIZER_JSON_CREATE = 0x01 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Failed to create JSON object in serializer */
  SC_SERIALIZER_NULL = 0x02 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< NULL object in serializer */
  SC_SERIALIZER_JSON_PARSE = 0x03 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Failed to parse JSON object in serializer */
  SC_SERIALIZER_JSON_PARSE_ASCII = 0x04 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Unicode value in JSON */
  SC_SERIALIZER_INVALID_REQ = 0x05 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Invald request value in JSON */
  SC_SERIALIZER_MESSAGE_OVERRUN = 0x06 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Message length is out of valid size */

  // Cache module
  SC_CACHE_NULL = 0x01 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< NULL object in cache */
  SC_CACHE_FAILED_RESPONSE = 0x02 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< Failed in cache operations */
  SC_CACHE_OFF = 0x03 | SC_MODULE_CACHE | SC_SEVERITY_MINOR,
  /**< Cache server is not turned on */
  SC_CACHE_INIT_FINI = 0x04 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< Failed to initialize or destroy lock in cache */
  SC_CACHE_LOCK_FAILURE = 0x05 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< Failed to lock or unlock cache operations */

  // MAM module
  SC_MAM_NULL = 0x01 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< NULL object in mam */
  SC_MAM_NOT_FOUND = 0x02 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Empty result from mam */
  SC_MAM_FAILED_INIT = 0x03 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Error in mam initialization */
  SC_MAM_FAILED_RESPONSE = 0x04 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Error in mam response */
  SC_MAM_FAILED_DESTROYED = 0x05 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Error in mam destroy */
  SC_MAM_NO_PAYLOAD = 0x06 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< No payload in the MAM message with assigning IDs */
  SC_MAM_FAILED_WRITE = 0x07 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Failed to write in MAM */
  SC_MAM_FILE_SAVE = 0x08 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Failed to save MAM file */
  SC_MAM_ALL_MSS_KEYS_USED = 0x09 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< All MSS private keys of current given parameters are used */
  SC_MAM_FAILED_CREATE_OR_GET_ID = 0x0A | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Failed to created/get chid or epid or msg_id in MAM */
  SC_MAM_FAILED_WRITE_HEADER = 0x0B | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Failed to write header in MAM */
  SC_MAM_MESSAGE_NOT_FOUND = 0x0C | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Can't find message in the assign bundle */
  SC_MAM_INVAID_CHID_OR_EPID = 0x0E | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Failed to add trusted channel ID or endpoint ID */

  // response module
  SC_RES_NULL = 0x01 | SC_MODULE_RES | SC_SEVERITY_FATAL,
  /**< NULL object in response */

  // configuration module
  SC_CONF_NULL = 0x01 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< NULL object in configuration */
  SC_CONF_MISSING_ARGUMENT = 0x02 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< No argument in CLI */
  SC_CONF_UNKNOWN_OPTION = 0x03 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< undefined option in CLI */
  SC_CONF_LOCK_INIT = 0x04 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< Failed to init lock */
  SC_CONF_LOCK_DESTROY = 0x05 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< Failed to destroy lock */
  SC_CONF_PARSER_ERROR = 0x06 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< Failed to initialize yaml parser */
  SC_CONF_FOPEN_ERROR = 0x07 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< Failed to open file */

  // UTILS module
  /**< NULL object in utils */
  SC_UTILS_WRONG_REQUEST_OBJ = 0x01 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Wrong TA request object */
  SC_UTILS_TIMER_ERROR = 0x02 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Errors occurred in timer function */
  SC_UTILS_TIMER_EXPIRED = 0x03 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Failed to send message */
  SC_UTILS_HTTPS_SEND_ERROR = 0x04 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< HTTPS module initialize error */
  SC_UTILS_HTTPS_INIT_ERROR = 0x05 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< HTTPS X509 certificate parse error */
  SC_UTILS_HTTPS_X509_ERROR = 0x06 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< HTTPS initial connection error */
  SC_UTILS_HTTPS_CONN_ERROR = 0x07 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< HTTPS setting SSL config error */
  SC_UTILS_HTTPS_SSL_ERROR = 0x08 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< HTTPS response error */
  SC_UTILS_HTTPS_RESPONSE_ERROR = 0x09 | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Error occurred when serializing message */
  SC_UTILS_TEXT_SERIALIZE = 0x0A | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Error occurred when deserializing message */
  SC_UTILS_TEXT_DESERIALIZE = 0x0B | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Out of memory error */
  SC_UTILS_OOM_ERROR = 0x0C | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Overflow error */
  SC_UTILS_OVERFLOW_ERROR = 0x0D | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Error occurred when encrypting or descrypting message */
  SC_UTILS_CIPHER_ERROR = 0x0E | SC_MODULE_UTILS | SC_SEVERITY_FATAL,
  /**< Timer expired */

  // HTTP module
  SC_HTTP_OOM = 0x01 | SC_MODULE_HTTP | SC_SEVERITY_FATAL,
  /**< Failed to create http object */
  SC_HTTP_NULL = 0x02 | SC_MODULE_HTTP | SC_SEVERITY_FATAL,
  /**< NULL object in http */
  SC_HTTP_INVALID_REGEX = 0x03 | SC_MODULE_HTTP | SC_SEVERITY_MAJOR,
  /**< Invalid URL regular expression rule in http */
  SC_HTTP_URL_NOT_MATCH = 0x04 | SC_MODULE_HTTP | SC_SEVERITY_MAJOR,
  /**< URL doesn't match regular expression rule */
  SC_HTTP_URL_PARSE_ERROR = 0x05 | SC_MODULE_HTTP | SC_SEVERITY_MAJOR,
  /**< Proxy API command not match */
  SC_HTTP_COMMAND_NOT_MATCH = 0x06 | SC_MODULE_HTTP | SC_SEVERITY_MAJOR,
  /**< URL parameter parsing error */

  // MQTT module
  SC_MQTT_OOM = 0x01 | SC_MODULE_MQTT | SC_SEVERITY_FATAL,
  /**< Failed to create MQTT object */
  SC_MQTT_NULL = 0x02 | SC_MODULE_MQTT | SC_SEVERITY_FATAL,
  /**< NULL object in MQTT */
  SC_MQTT_INIT = 0x03 | SC_MODULE_MQTT | SC_SEVERITY_MAJOR,
  /**< Error during initialization in MQTT */
  SC_MOSQ_OBJ_INIT_ERROR = 0x04 | SC_MODULE_MQTT | SC_SEVERITY_MAJOR,
  /**< Error in initializing mosquitto object */
  SC_MQTT_TOPIC_SET = 0x05 | SC_MODULE_MQTT | SC_SEVERITY_MAJOR,
  /**< Error in setting topic in MQTT */
  SC_MQTT_OPT_SET = 0x06 | SC_MODULE_MQTT | SC_SEVERITY_MAJOR,
  /**< Error in setting options of `struct mosquitto` object */
  SC_CLIENT_CONNECT = 0x07 | SC_MODULE_MQTT | SC_SEVERITY_MAJOR,
  /**< Error in connecting to broker */
  SC_MQTT_INVALID_TAG = 0x08 | SC_MODULE_MQTT | SC_SEVERITY_MAJOR,
  /**< Received invalid tag length in MQTT */

  // STORAGE module
  SC_STORAGE_OOM = 0x01 | SC_MODULE_STORAGE | SC_SEVERITY_FATAL,
  /**< Failed to create storage object */
  SC_STORAGE_CONNECT_FAIL = 0x02 | SC_MODULE_STORAGE | SC_SEVERITY_MAJOR,
  /**< Failed to connect ScyllaDB node */
  SC_STORAGE_INVALID_INPUT = 0x03 | SC_MODULE_STORAGE | SC_SEVERITY_MAJOR,
  /**< Invalid input parameter, e.g., null pointer */
  SC_STORAGE_CASSANDRA_QUERY_FAIL = 0x04 | SC_MODULE_STORAGE | SC_SEVERITY_MAJOR,
  /**< Failed to execute Cassandra query */

  // Core module
  SC_CORE_OOM = 0x01 | SC_MODULE_CORE | SC_SEVERITY_FATAL,
  /**< Failed to create core object */
  SC_CORE_NULL = 0x02 | SC_MODULE_CORE | SC_SEVERITY_FATAL,
  /**< NULL object in core */
  SC_CORE_IRI_UNSYNC = 0x03 | SC_MODULE_CORE | SC_SEVERITY_FATAL,
  /**< IRI host is not synchronized */

  // Endpoint module
  /**< Failed to initialize the device  */
  SC_ENDPOINT_DEVICE_INIT = 0x01 | SC_MODULE_ENDPOINT | SC_SEVERITY_FATAL,
  /**< Failed to finalize the device */
  SC_ENDPOINT_DEVICE_FINI = 0x02 | SC_MODULE_ENDPOINT | SC_SEVERITY_FATAL,
  /**< Uart error occurred in device component */
  SC_ENDPOINT_UART = 0x03 | SC_MODULE_ENDPOINT | SC_SEVERITY_FATAL,
  /**< Error occurred inside secure storage */
  SC_ENDPOINT_SEC_FAULT = 0x04 | SC_MODULE_ENDPOINT | SC_SEVERITY_MINOR,
  /**< Error occurred when item not found inside secure storage */
  SC_ENDPOINT_SEC_ITEM_NOT_FOUND = 0x05 | SC_MODULE_ENDPOINT | SC_SEVERITY_MINOR,
  /**< Error occurred when the secure storage service is unavailable */
  SC_ENDPOINT_SEC_UNAVAILABLE = 0x06 | SC_MODULE_ENDPOINT | SC_SEVERITY_MINOR,
  /**< Error occurred when the sending transfer message */
  SC_ENDPOINT_SEND_TRANSFER = 0x07 | SC_MODULE_ENDPOINT | SC_SEVERITY_FATAL,

} status_t;

typedef enum {
  HTTP_METHOD_GET = 0,     /**< HTTP GET method */
  HTTP_METHOD_POST = 1,    /**< HTTP POST method */
  HTTP_METHOD_OPTIONS = 2, /**< HTTP OPTIONS method */
} http_method_t;

/**
 * @brief error code to string
 *
 * @param err error code
 * @return string
 */
const char* ta_error_to_string(status_t err);

#ifdef __cplusplus
}
#endif

#endif  // COMMON_TA_ERRORS_H_
