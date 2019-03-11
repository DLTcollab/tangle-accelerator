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

#define SC_SEVERITY_MINOR (0x0 << SC_SEVERITY_MASK)
#define SC_SEVERITY_MODERATE (0x01 << SC_SEVERITY_MASK)
#define SC_SEVERITY_MAJOR (0x02 << SC_SEVERITY_MASK)
#define SC_SEVERITY_FATAL (0x03 << SC_SEVERITY_MASK)
/** @} */

/** @name module code */
/** @{ */
#define SC_MODULE_MASK 0xE0
#define SC_MODULE_SHIFT 5

#define SC_MODULE_TA (0x01 << SC_MODULE_SHIFT)
#define SC_MODULE_CCLIENT (0x02 << SC_MODULE_SHIFT)
#define SC_MODULE_SERIALIZER (0x03 << SC_MODULE_SHIFT)
#define SC_MODULE_CACHE (0x04 << SC_MODULE_SHIFT)
/** @} */

/** @name serverity code */
/** @{ */
#define SC_ERROR_MASK 0x07
/** @} */

/** @name http error code */
/** @{ */
#define SC_BAD_REQUEST 400
#define SC_NOT_FOUND 404
/** @} */

/* status code */
typedef enum {
  SC_OK = 0, /**< No error */

  SC_TA_OOM = 0x01 | SC_MODULE_TA | SC_SEVERITY_FATAL,
  /**< Fail to create TA object */
  SC_TA_NULL = 0x02 | SC_MODULE_TA | SC_SEVERITY_FATAL,
  /**< NULL TA objects */

  // CClient module
  SC_CCLIENT_OOM = 0x01 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Fail to create cclient object */
  SC_CCLIENT_NOT_FOUND = 0x02 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Empty result from cclient */
  SC_CCLIENT_FAILED_RESPONSE = 0x03 | SC_MODULE_CCLIENT | SC_SEVERITY_FATAL,
  /**< Error in cclient response */
  SC_CCLIENT_INVALID_FLEX_TRITS = 0x04 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< flex_trits conversion error */
  SC_CCLIENT_HASH = 0x05 | SC_MODULE_CCLIENT | SC_SEVERITY_MAJOR,
  /**< hash container operation error */

  // Serializer module
  SC_SERIALIZER_JSON_CREATE = 0x01 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Fail to create JSON object in serializer */
  SC_SERIALIZER_JSON_PARSE = 0x02 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< Fail to parse JSON object in serializer */
  SC_SERIALIZER_NULL = 0x03 | SC_MODULE_SERIALIZER | SC_SEVERITY_FATAL,
  /**< NULL object in serializer */

  // Cache module
  SC_CACHE_NULL = 0x01 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< NULL parameters in cache */
  SC_CACHE_FAILED_RESPONSE = 0x02 | SC_MODULE_CACHE | SC_SEVERITY_FATAL,
  /**< Fail in cache operations */
} status_t;

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_ERRORS_H_
