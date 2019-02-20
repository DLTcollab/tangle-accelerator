#ifndef ACCELERATOR_CONFIG_H_
#define ACCELERATOR_CONFIG_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file config.h
 * @brief Configuration of tangle-acclerator
 */

/** @name tangle-accelerator config */
/** @{ */
#define TA_HOST "localhost" /**< Binding address of tangle-acclerator */
#define TA_PORT "8000"      /**< Binding port of tangle-acclerator */
#define TA_THREAD_COUNT 10  /**< Thread count of tangle-acclerator instance */
/** @} */

/** @name IRI connection config */
/** @{ */
#define IRI_HOST "localhost" /**< Address of IRI */
#define IRI_PORT 14265       /**< Port of IRI */
#define DEPTH 3              /**< Depth of API argument */
#define MWM 14               /**< Maximum weight magnitude of API argument */
/** Seed to generate address. This does not do any signature yet. */
#define SEED                                                                   \
  "AMRWQP9BUMJALJHBXUCHOD9HFFD9LGTGEAWMJWWXSDVOF9PI9YGJAPBQLQUOMNYEQCZPGCTHGV" \
  "NNAPGHA"
/** @} */

/** @name Redis connection config */
/** @{ */
#define REDIS_HOST "localhost" /**< Address of Redis server */
#define REDIS_PORT 6379        /**< poer of Redis server */
/** @} */

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_CONFIG_H_
