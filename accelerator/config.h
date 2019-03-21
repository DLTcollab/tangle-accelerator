#ifndef ACCELERATOR_CONFIG_H_
#define ACCELERATOR_CONFIG_H_

#include "accelerator/errors.h"
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "cclient/types/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file config.h
 * @brief Configuration of tangle-accelerator
 */

#define TA_VERSION "tangle-accelerator/0.3.0"
#define TA_HOST "localhost"
#define TA_PORT "8000"
#define TA_THREAD_COUNT 10
#define IRI_HOST "localhost"
#define IRI_PORT 14265
#define DEPTH 3
#define MWM 14
#define SEED                                                                   \
  "AMRWQP9BUMJALJHBXUCHOD9HFFD9LGTGEAWMJWWXSDVOF9PI9YGJAPBQLQUOMNYEQCZPGCTHGV" \
  "NNAPGHA"

/** @name Redis connection config */
/** @{ */
#define REDIS_HOST "localhost" /**< Address of Redis server */
#define REDIS_PORT 6379        /**< poer of Redis server */
/** @} */

/** struct type of accelerator configuration */
typedef struct ta_info_s {
  char* version;        /**< Binding version of tangle-accelerator */
  char* host;           /**< Binding address of tangle-accelerator */
  char* port;           /**< Binding port of tangle-accelerator */
  uint8_t thread_count; /**< Thread count of tangle-accelerator instance */
} ta_config_t;

/** struct type of iota configuration */
typedef struct ta_config_s {
  uint8_t depth; /**< Depth of API argument */
  uint8_t mwm;   /**< Minimum weight magnitude of API argument */
  /** Seed to generate address. This does not do any signature yet. */
  const char* seed;
} iota_config_t;

/** struct type of accelerator core */
typedef struct ta_core_s {
  ta_config_t info;              /**< accelerator configiuration structure */
  iota_config_t config;          /**< iota configuration structure */
  iota_client_service_t service; /**< iota connection structure */
} ta_core_t;

/**
 * Initializes configurations with default values
 * Should be called first
 *
 * @param info[in] Tangle-accelerator configuration variables
 * @param config[in] iota configuration variables
 * @param service[in] IRI connection configuration variables
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_config_init(ta_config_t* const info, iota_config_t* const config,
                        iota_client_service_t* const service);

/**
 * Free memory of configuration variables
 *
 * @param service[in] IRI connection configuration variables
 */
void ta_config_destroy(iota_client_service_t* const service);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_CONFIG_H_
