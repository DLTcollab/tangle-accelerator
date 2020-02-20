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

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/cli_info.h
 * @brief Message and options for tangled-accelerator configures
 */

typedef enum ta_cli_arg_value_e {
  /** TA */
  TA_HOST_CLI = 127,
  TA_PORT_CLI,
  TA_THREAD_COUNT_CLI,

  /** IRI */
  IRI_HOST_CLI,
  IRI_PORT_CLI,

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
  GTTA_DISABLE,

  /** LOGGER */
  QUIET,
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
    {"ta_thread", optional_argument, NULL, TA_THREAD_COUNT_CLI, "TA executing thread"},
    {"iri_host", required_argument, NULL, IRI_HOST_CLI, "IRI listening host"},
    {"iri_port", required_argument, NULL, IRI_PORT_CLI, "IRI listening port"},
    {"mqtt_host", required_argument, NULL, MQTT_HOST_CLI, "MQTT listening host"},
    {"mqtt_root", required_argument, NULL, MQTT_ROOT_CLI, "MQTT listening topic root"},
    {"redis_host", required_argument, NULL, REDIS_HOST_CLI, "Redis server listening host"},
    {"redis_port", required_argument, NULL, REDIS_PORT_CLI, "Redis server listening port"},
    {"db_host", required_argument, NULL, DB_HOST_CLI, "DB server listening host"},
    {"milestone_depth", optional_argument, NULL, MILESTONE_DEPTH_CLI, "IRI milestone depth"},
    {"mwm", optional_argument, NULL, MWM_CLI, "minimum weight magnitude"},
    {"seed", optional_argument, NULL, SEED_CLI, "IOTA seed"},
    {"cache", required_argument, NULL, CACHE, "Enable cache server with Y"},
    {"config", required_argument, NULL, CONF_CLI, "Read configuration file"},
    {"proxy_passthrough", no_argument, NULL, PROXY_API, "Pass proxy API directly to IRI without processing"},
    {"gtta_disable", no_argument, NULL, GTTA_DISABLE, "Disable GTTA when sending transacation"},
    {"quiet", no_argument, NULL, QUIET, "Disable logger"},
    {NULL, 0, NULL, 0, NULL}};

static const int cli_cmd_num = sizeof(ta_cli_arguments_g) / sizeof(struct ta_cli_argument_s);

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
