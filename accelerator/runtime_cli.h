/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#ifndef ACCELERATOR_RUNTIME_CLI_H
#define ACCELERATOR_RUNTIME_CLI_H

#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @file accelerator/runtime_cli.h
 * @brief Runtime command-line utility
 */

/**
 * @brief Runtime command-line utility thread
 */
void *cli_routine(void *arg);

/**
 * @brief Command state types in state machine
 */
enum {
  CMD_LAST,  /**< The last subcommand, could be followed by zero or more parameters */
  CMD_STATE, /**< Subcommands expected, should step to next state according to `next_state` */
};
struct cli_cmd;
struct cli_list;

/**
 * @brief Structure that holds the information of a (sub)command.
 */
typedef struct cli_cmd {
  const char *name;     /**< Name of the command */
  const char *describe; /**< Detailed description of the command */
  const char *example;  /**< Example usage of the command (should be empty string literal if not needed */
  const int type;       /**< Command state types */
  union {
    struct cli_list *next_state; /**< Determine the next state to step (CMD_STATE) */
    void (*fn)(char *cmd);       /**< Determine the function to call (CMD_LAST) */
  };
} ta_cli_cmd;

/**
 * @brief Structure that holds the information of a state
 *
 * We define the different states using `ta_cli_list_t`,
 * each state can determine whether to step to next state (CMD_STATE)
 * or stop (CMD_LAST).
 */
typedef struct cli_list {
  const size_t size;           /**< Number of (sub)commands in the state */
  const struct cli_cmd list[]; /**< Array of (sub)commands */
} ta_cli_list_t;

#ifdef __cplusplus
}
#endif

#endif
