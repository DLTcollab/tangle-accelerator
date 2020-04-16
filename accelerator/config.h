/*
 * Copyright (C) 2018-2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_CONFIG_H_
#define ACCELERATOR_CONFIG_H_

#include <ctype.h>
#include <getopt.h>

#include "accelerator/cli_info.h"
#include "accelerator/core/pow.h"
#include "cclient/api/core/core_api.h"
#include "cclient/api/extended/extended_api.h"
#ifdef DB_ENABLE
#include "storage/ta_storage.h"
#endif
#include "common/logger.h"
#include "utils/cache/cache.h"

#define FILE_PATH_SIZE 128

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/config.h
 * @brief Configuration of tangle-accelerator
 */

#define TA_VERSION "tangle-accelerator/0.9.3"
#define TA_HOST "localhost"

#ifdef MQTT_ENABLE
#define MQTT_HOST "localhost"
#define TOPIC_ROOT "root/topics"
#endif

#define TA_PORT 8000
#define TA_THREAD_COUNT 10
#define IRI_HOST "localhost"
#define IRI_PORT 14265
#define MAX_IRI_LIST_ELEMENTS 5
#define DB_HOST "localhost"
#define MILESTONE_DEPTH 3
#define MWM 14
#define SEED                                                                   \
  "AMRWQP9BUMJALJHBXUCHOD9HFFD9LGTGEAWMJWWXSDVOF9PI9YGJAPBQLQUOMNYEQCZPGCTHGV" \
  "NNAPGHA"
#define MAM_FILE_PREFIX "/tmp/mam_bin_XXXXXX"
#define IRI_HEALTH_TRACK_PERIOD 1800  // Check every half hour in default

/** @name Redis connection config */
/** @{ */
#define REDIS_HOST "localhost" /**< Address of Redis server */
#define REDIS_PORT 6379        /**< port of Redis server */
/** @} */

/** struct type of accelerator configuration */
typedef struct ta_config_s {
  char* version;                          /**< Binding version of tangle-accelerator */
  char* host;                             /**< Binding address of tangle-accelerator */
  int port;                               /**< Binding port of tangle-accelerator */
  char* host_list[MAX_IRI_LIST_ELEMENTS]; /**< List of binding host of tangle-accelerator */
  int port_list[MAX_IRI_LIST_ELEMENTS];   /**< List of binding port of tangle-accelerator */
  int health_track_period;                /**< The period for checking IRI host connection status */
#ifdef MQTT_ENABLE
  char* mqtt_host;       /**< Address of MQTT broker host */
  char* mqtt_topic_root; /**< The topic root of MQTT topic */
#endif
  uint8_t thread_count;   /**< Thread count of tangle-accelerator instance */
  bool proxy_passthrough; /**< Pass proxy api directly without processing */
  bool gtta;              /**< The option to turn on or off GTTA. The default value is true which enabling GTTA */
} ta_config_t;

/** struct type of iota configuration */
typedef struct iota_config_s {
  uint8_t milestone_depth;   /**< Depth of API argument */
  uint8_t mwm;               /**< Minimum weight magnitude of API argument */
  const char* seed;          /**< Seed to generate address. This does not do any signature yet. */
  const char* mam_file_path; /**< The MAM file which records the mam config */
} iota_config_t;

/** struct type of accelerator cache */
typedef struct ta_cache_s {
  char* host;       /**< Binding address of redis server */
  uint16_t port;    /**< Binding port of redis server */
  bool cache_state; /**< Set it true to turn on cache server */
} ta_cache_t;

/** struct type of accelerator core */
typedef struct ta_core_s {
  ta_config_t ta_conf;                /**< accelerator configiuration structure */
  ta_cache_t cache;                   /**< redis configiuration structure */
  iota_config_t iota_conf;            /**< iota configuration structure */
  iota_client_service_t iota_service; /**< iota connection structure */
#ifdef DB_ENABLE
  db_client_service_t db_service; /**< db connection structure */
#endif
  char conf_file[FILE_PATH_SIZE]; /**< path to the configuration file */
} ta_core_t;

/**
 * Initializes configurations with default values
 *
 * @param core[in] Pointer to Tangle-accelerator core configuration structure
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_core_default_init(ta_core_t* const core);

static inline struct option* cli_build_options() {
  struct option* long_options = (struct option*)malloc(cli_cmd_num * sizeof(struct option));
  for (int i = 0; i < cli_cmd_num; ++i) {
    long_options[i].name = ta_cli_arguments_g[i].name;
    long_options[i].has_arg = ta_cli_arguments_g[i].has_arg;
    long_options[i].flag = ta_cli_arguments_g[i].flag;
    long_options[i].val = ta_cli_arguments_g[i].val;
  }
  return long_options;
};

status_t cli_core_set(ta_core_t* const core, int key, char* const value);

/**
 * Initializes configurations with configuration file
 *
 * @param core[in] Pointer to Tangle-accelerator core configuration structure
 * @param argc[in] Number of argument of CLI
 * @param argv[in] Argument of CLI
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_core_file_init(ta_core_t* const core, int argc, char** argv);

/**
 * Initializes configurations with CLI values
 *
 * @param core[in] Pointer to Tangle-accelerator core configuration structure
 * @param argc[in] Number of argument of CLI
 * @param argv[in] Argument of CLI
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_core_cli_init(ta_core_t* const core, int argc, char** argv);

/**
 * Start services after configurations are set
 *
 * @param core[in] Pointer to Tangle-accelerator core configuration structure
 *
 * @return
 * - SC_OK on success
 * - non-zero on error
 */
status_t ta_core_set(ta_core_t* const core);

/**
 * Free memory of configuration variables
 *
 * @param core[in] Pointer to Tangle-accelerator core configuration structure.
 *
 */
void ta_core_destroy(ta_core_t* const core);

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_CONFIG_H_
