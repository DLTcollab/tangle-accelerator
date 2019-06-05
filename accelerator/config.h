#ifndef ACCELERATOR_CONFIG_H_
#define ACCELERATOR_CONFIG_H_

#include <getopt.h>

#include "accelerator/message.h"
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#include "utils/cache.h"
#include "utils/pow.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file config.h
 * @brief Configuration of tangle-accelerator
 */

#define TA_VERSION "tangle-accelerator/0.5.0"
#define TA_HOST "localhost"
#define TA_PORT "8000"
#define TA_THREAD_COUNT 10
#define IRI_HOST "localhost"
#define IRI_PORT 14265
#define MILESTONE_DEPTH 3
#define FSIZE 11
#define MSS_DEPTH 4
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
  uint8_t milestone_depth; /**< Depth of API argument */
  char mam_file[FSIZE];    /** Save file for mam struct like MSS, skn... */
  uint8_t mss_depth;       /**< Depth of MSS layer merkle tree */
  uint8_t mwm;             /**< Minimum weight magnitude of API argument */
  /** Seed to generate address. This does not do any signature yet. */
  const char* seed;
} iota_config_t;

/** struct type of accelerator cache */
typedef struct ta_cache_s {
  char* host;    /**< Binding address of redis server */
  uint16_t port; /**< Binding port of redis server */
} ta_cache_t;

/** struct type of accelerator core */
typedef struct ta_core_s {
  ta_config_t info;              /**< accelerator configiuration structure */
  ta_cache_t cache;              /**< redis configiuration structure */
  iota_config_t tangle;          /**< iota configuration structure */
  iota_client_service_t service; /**< iota connection structure */
} ta_core_t;

/**
 * Initializes configurations with default values
 * Should be called first
 *
 * @param info[in] Tangle-accelerator configuration variables
 * @param tangle[in] iota configuration variables
 * @param cache[in] redis configuration variables
 * @param service[in] IRI connection configuration variables
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_config_default_init(ta_config_t* const info,
                                iota_config_t* const tangle,
                                ta_cache_t* const cache,
                                iota_client_service_t* const service);

/**
 * Initializes configurations with CLI values
 * Should be called third
 *
 * @param ta_conf[in] All configuration variables
 * @param argc[in] Number of argumentof CLI
 * @param argv[in] Argument of CLI
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_config_cli_init(ta_core_t* const ta_conf, int argc, char** argv);

/**
 * Start services after configurations are set
 *
 * @param cache[in] Redis server configuration variables
 * @param service[in] IRI connection configuration variables
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_config_set(ta_cache_t* const cache,
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
