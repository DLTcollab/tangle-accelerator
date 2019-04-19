#ifndef ACCELERATOR_ERRORS_H_
#define ACCELERATOR_ERRORS_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file errors.h
 * @brief Error Code of tangle-acclerator
 *
 * bit division:
 * 0 - 3 actual error code
 * 4 - 5 serverity
 * 6 - 8 error module
 *
 * *--------*
 * |MMMSSCCC|
 * *--------*
 */

/** @name serverity code */
/** @{ */
#define SC_SEVERITY_MASK 0x18
#define SC_SEVERITY_SHIFT 3

#define SC_SEVERITY_MINOR (0x0 << SC_SEVERITY_SHIFT)
#define SC_SEVERITY_MODERATE (0x01 << SC_SEVERITY_SHIFT)
#define SC_SEVERITY_MAJOR (0x02 << SC_SEVERITY_SHIFT)
#define SC_SEVERITY_FATAL (0x03 << SC_SEVERITY_SHIFT)
/** @} */

/** @name module code */
/** @{ */
#define SC_MODULE_MASK 0xE0
#define SC_MODULE_SHIFT 5

#define SC_MODULE_TA (0x01 << SC_MODULE_SHIFT)
#define SC_MODULE_CCLIENT (0x02 << SC_MODULE_SHIFT)
#define SC_MODULE_SERIALIZER (0x03 << SC_MODULE_SHIFT)
#define SC_MODULE_CACHE (0x04 << SC_MODULE_SHIFT)
#define SC_MODULE_MAM (0x05 << SC_MODULE_SHIFT)
#define SC_MODULE_RES (0x06 << SC_MODULE_SHIFT)
#define SC_MODULE_CONF (0x07 << SC_MODULE_SHIFT)
/** @} */

/** @name serverity code */
/** @{ */
#define SC_ERROR_MASK 0x07
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
  /**< Fail to create TA object */
  SC_TA_NULL = 0x02 | SC_MODULE_TA | SC_SEVERITY_FATAL,
  /**< NULL TA objects */

  // CClient module
  SC_CCLIENT_OOM = 0x01 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Fail to create cclient object */
  SC_CCLIENT_NOT_FOUND = 0x03 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Empty result from cclient */
  SC_CCLIENT_FAILED_RESPONSE = 0x04 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Error in cclient response */
  SC_CCLIENT_INVALID_FLEX_TRITS = 0x05 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< flex_trits conversion error */
  SC_CCLIENT_HASH = 0x06 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< hash container operation error */
  SC_CCLIENT_JSON_KEY = 0x07 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< JSON key not found */
  SC_CCLIENT_JSON_PARSE = 0x08 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< json parsing error, might the wrong format */

  // Serializer module
  SC_SERIALIZER_JSON_CREATE = 0x01 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Fail to create JSON object in serializer */
  SC_SERIALIZER_NULL = 0x02 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< NULL object in serializer */
  SC_SERIALIZER_JSON_PARSE = 0x03 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Fail to parse JSON object in serializer */

  // Cache module
  SC_CACHE_NULL = 0x02 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< NULL parameters in cache */
  SC_CACHE_FAILED_RESPONSE = 0x04 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< Fail in cache operations */

  // MAM module
  SC_MAM_OOM = 0x01 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Fail to create mam object */
  SC_MAM_NULL = 0x02 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< NULL object in mam */
  SC_MAM_NOT_FOUND = 0x03 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Empty result from mam */
  SC_MAM_FAILED_INIT = 0x04 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Error in mam initialization */
  SC_MAM_FAILED_RESPONSE = 0x05 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Error in mam response */
  SC_MAM_FAILED_DESTROYED = 0x06 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Error in mam destroy */
  SC_MAM_NO_PAYLOAD = 0x07 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< No payload or no chid */
  SC_MAM_FAILED_WRITE = 0x08 | SC_MODULE_MAM | SC_SEVERITY_FATAL,
  /**< Failed to write */

  // response module
  SC_RES_OOM = 0x01 | SC_MODULE_RES | SC_SEVERITY_FATAL,
  /**< Fail to create response object */
  SC_RES_NULL = 0x02 | SC_MODULE_RES | SC_SEVERITY_FATAL,
  /**< NULL object in response */

  // configuration module
  SC_CONF_NULL = 0x02 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< NULL object in response */
  SC_CONF_MISSING_ARGUMENT = 0x04 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< No argument in CLI */
  SC_CONF_UNKNOWN_OPTION = 0x05 | SC_MODULE_CONF | SC_SEVERITY_FATAL,
  /**< undefined option in CLI */
} status_t;

typedef enum {
  HTTP_METHOD_GET = 0,     /**< HTTP GET method */
  HTTP_METHOD_POST = 1,    /**< HTTP POST method */
  HTTP_METHOD_OPTIONS = 2, /**< HTTP OPTIONS method */
} http_method_t;

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_ERRORS_H_
