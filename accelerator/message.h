/*
 * Copyright (C) 2018-2019 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_MESSAGE_H_
#define ACCELERATOR_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/message.h
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

  /** REDIS */
  REDIS_HOST_CLI,
  REDIS_PORT_CLI,

  /** CONFIG */
  MILESTONE_DEPTH_CLI,
  MWM_CLI,
  SEED_CLI,
  CACHE,

  /** LOGGER */
  VERBOSE,
} ta_cli_arg_value_t;

typedef enum ta_cli_arg_requirement_e { NO_ARG, REQUIRED_ARG, OPTIONAL_ARG } ta_cli_arg_requirement_t;

static struct ta_cli_argument_s {
  char const* name;
  int val;
  char const* desc;
  ta_cli_arg_requirement_t has_arg;
} ta_cli_arguments_g[] = {{"help", 'h', "Show tangle-accelerator usage", NO_ARG},
                          {"version", 'v', "tangle-accelerator version", NO_ARG},
                          {"ta_host", TA_HOST_CLI, "TA listening host", REQUIRED_ARG},
                          {"ta_port", TA_PORT_CLI, "TA listening port", REQUIRED_ARG},
                          {"ta_thread", TA_THREAD_COUNT_CLI, "TA executing thread", OPTIONAL_ARG},
                          {"iri_host", IRI_HOST_CLI, "IRI listening host", REQUIRED_ARG},
                          {"iri_port", IRI_PORT_CLI, "IRI listening port", REQUIRED_ARG},
                          {"redis_host", REDIS_HOST_CLI, "Redis server listening host", REQUIRED_ARG},
                          {"redis_port", REDIS_PORT_CLI, "Redis server listening port", REQUIRED_ARG},
                          {"milestone_depth", MILESTONE_DEPTH_CLI, "IRI milestone depth", OPTIONAL_ARG},
                          {"mwm", MWM_CLI, "minimum weight magnitude", OPTIONAL_ARG},
                          {"seed", SEED_CLI, "IOTA seed", OPTIONAL_ARG},
                          {"cache", CACHE, "Enable cache server with Y", REQUIRED_ARG},
                          {"verbose", VERBOSE, "Enable logger", NO_ARG}};

static const int cli_cmd_num = sizeof(ta_cli_arguments_g) / sizeof(struct ta_cli_argument_s);
void ta_usage();
#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_MESSAGE_H_
