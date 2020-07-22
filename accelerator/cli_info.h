/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_CLI_INFO_H_
#define ACCELERATOR_CLI_INFO_H_

#include <stdio.h>
#include "common/macros.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/cli_info.h
 * @brief Message and options for tangle-accelerator configuration
 */

typedef enum ta_cli_arg_value_e {
  /** tangle-accelerator */
  TA_HOST_CLI = 127,
  TA_PORT_CLI,
  RUNTIME_CLI,

  /** IOTA full node */
  NODE_HOST_CLI,
  NODE_PORT_CLI,
  NODE_ADDRESS_CLI,

  /** MQTT */
  MQTT_HOST_CLI,
  MQTT_ROOT_CLI,

  /** REDIS */
  REDIS_HOST_CLI,
  REDIS_PORT_CLI,

  /** DB */
  DB_HOST_CLI,

  /** CONFIG */
  MILESTONE_DEPTH_CLI,
  MWM_CLI,
  SEED_CLI,
  CACHE,
  CONF_CLI,
  PROXY_API,
  HEALTH_TRACK_PERIOD,
  NO_GTTA,
  BUFFER_LIST,
  COMPLETE_LIST,
  HTTP_THREADS_CLI,
  CACHE_CAPACITY,
  IPC,

  /** LOGGER */
  QUIET,

  CA_PEM,
} ta_cli_arg_value_t;

static struct ta_cli_argument_s {
  char const* name;
  int has_arg; /* one of: no_argument, required_argument, optional_argument */
  int* flag;
  int val;
  const char* desc;
} ta_cli_arguments_g[] = {
    {"help", no_argument, NULL, 'h', "Show tangle-accelerator usage"},
    {"version", no_argument, NULL, 'v', "tangle-accelerator version"},
    {"ta_host", required_argument, NULL, TA_HOST_CLI, "TA listening host"},
    {"ta_port", required_argument, NULL, TA_PORT_CLI, "TA listening port"},
    {"http_threads", required_argument, NULL, HTTP_THREADS_CLI,
     "Determine thread pool size to process HTTP connections."},
    {"node_host", required_argument, NULL, NODE_HOST_CLI, "IOTA full node listening host"},
    {"node_port", required_argument, NULL, NODE_PORT_CLI, "IOTA full node listening port"},
    {"CA_PEM", required_argument, NULL, CA_PEM, "The path to CA PEM file"},
    {"mqtt_host", required_argument, NULL, MQTT_HOST_CLI, "MQTT listening host"},
    {"mqtt_root", required_argument, NULL, MQTT_ROOT_CLI, "MQTT listening topic root"},
    {"node_address", required_argument, NULL, NODE_ADDRESS_CLI, " List of IOTA full node listening URL"},
    {"redis_host", required_argument, NULL, REDIS_HOST_CLI, "Redis server listening host"},
    {"redis_port", required_argument, NULL, REDIS_PORT_CLI, "Redis server listening port"},
    {"db_host", required_argument, NULL, DB_HOST_CLI, "DB server listening host"},
    {"milestone_depth", optional_argument, NULL, MILESTONE_DEPTH_CLI, "IOTA full node milestone depth"},
    {"mwm", optional_argument, NULL, MWM_CLI, "minimum weight magnitude"},
    {"seed", optional_argument, NULL, SEED_CLI, "IOTA seed"},
    {"cache", required_argument, NULL, CACHE, "Enable/Disable cache server. It defaults to off"},
    {"ipc", required_argument, NULL, IPC, "Set the socket name of initializing notification"},
    {"config", required_argument, NULL, CONF_CLI, "Read configuration file"},
    {"proxy_passthrough", no_argument, NULL, PROXY_API, "Pass proxy API directly to IOTA full node without processing"},
    {"health_track_period", required_argument, NULL, HEALTH_TRACK_PERIOD,
     "The period for checking IOTA full node host connection status"},
    {"no-gtta", no_argument, NULL, NO_GTTA, "Disable getTransactionToConfirm (gTTA) when sending transaction"},
    {"buffer_list", required_argument, NULL, BUFFER_LIST, "Set the value of `buffer_list_name`"},
    {"complete_list", required_argument, NULL, COMPLETE_LIST, "Set the value of `complete_list_name`"},
    {"cache_capacity", required_argument, NULL, CACHE_CAPACITY, "Set the maximum capacity of caching server"},
    {"quiet", no_argument, NULL, QUIET, "Disable logger"},
    {"runtime_cli", no_argument, NULL, RUNTIME_CLI, "Enable runtime command line"},
    {NULL, 0, NULL, 0, NULL}};

static const int cli_cmd_num = ARRAY_SIZE(ta_cli_arguments_g);

static inline void ta_usage() {
  printf("tangle-accelerator usage:\n");
  for (int i = 0; i < cli_cmd_num; i++) {
    printf("--%-34s ", ta_cli_arguments_g[i].name);
    printf("     ");
    if (ta_cli_arguments_g[i].has_arg == required_argument) {
      printf(" arg ");
    } else if (ta_cli_arguments_g[i].has_arg == optional_argument) {
      printf("[arg]");
    } else {
      printf("     ");
    }
    printf(" %s \n", ta_cli_arguments_g[i].desc);
  }
}

#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_CLI_INFO_H_
