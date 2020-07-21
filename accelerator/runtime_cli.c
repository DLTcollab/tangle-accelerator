/*
 * Copyright (C) 2020 BiiLabs Co., Ltd. and Contributors
 * All Rights Reserved.
 * This is free software; you can redistribute it and/or modify it under the
 * terms of the MIT license. A copy of the license can be found in the file
 * "LICENSE" at the root of this distribution.
 */

#include "runtime_cli.h"
#include <stdbool.h>
#include "config.h"
#include "utils/cache/cache.h"
#include "utils/macros.h"

static ta_core_t *ta_core;

/* Forward declaration for state definition and functions */
static ta_cli_list_t start_state, redis_state, redis_memory_state, logger_state;
static void do_help(char *);
static void do_logger_switch(char *);
static void do_logger_status(char *);
static void do_redis_memory_set(char *);
static void do_redis_memory_get(char *);
static void handle_command(char *);
static ta_cli_list_t *ta_state_ptr; /* Static global state pointer */

/* Thread for runtime command line utility */
void *cli_routine(void *arg) {
  ta_core = (ta_core_t *)arg;
  UNUSED(arg);
  char command[128];
  while (true) {
    printf("Command> ");
    fgets(command, 128, stdin);
    command[strlen(command) - 1] = '\0';
    if (strlen(command) == 0) continue;

    /* Initialize the state then process the command */
    ta_state_ptr = &start_state;
    handle_command(command);
  }
  return NULL;
}

/* Start of state definitions */
static ta_cli_list_t start_state = {
    .size = 2,
    {
        {
            .name = "logger",
            .type = CMD_STATE,
            .next_state = &logger_state,
            .describe = "Logger related operations",
            .example = "",
        },
        {
            .name = "redis",
            .type = CMD_STATE,
            .next_state = &redis_state,
            .describe = "Redis related operations",
            .example = "",
        },
    },
};
static ta_cli_list_t redis_state = {
    .size = 1,
    {
        {
            .name = "capacity",
            .type = CMD_STATE,
            .next_state = &redis_memory_state,
            .describe = "Redis memory capacity related operations",
            .example = "",
        },
    },
};
static ta_cli_list_t redis_memory_state = {
    .size = 2,
    {
        {
            .name = "get",
            .fn = do_redis_memory_get,
            .type = CMD_LAST,
            .describe = "Get Redis memory capacity",
            .example = "redis capacity get",
        },
        {
            .name = "set",
            .fn = do_redis_memory_set,
            .type = CMD_LAST,
            .describe = "Set Redis memory capacity",
            .example = "redis capacity set [limit]",
        },
    },
};
static ta_cli_list_t logger_state = {
    .size = 2,
    {
        {
            .name = "switch",
            .fn = do_logger_switch,
            .type = CMD_LAST,
            .describe = "Switch on/off logger",
            .example = "logger switch [on/off]",
        },
        {
            .name = "status",
            .fn = do_logger_status,
            .type = CMD_LAST,
            .describe = "Show current logger status",
            .example = "logger status",
        },
    },
};
/* End of state definitions */

/**
 * @brief Extract one subcommand from given string
 *
 * The orig parameter is sliced to be the remaining string, e.g.
 *
 * orig(input) = extracted + " " + orig(output)
 *
 * @param[in,out] orig The original string to be extracted
 * @param[out] extracted The string extracted from orig.
 *
 * @returns
 * - True if the given string can be separated to two strings
 * - False if the given string cannot be separated to two strings
 */
static bool extract_one(char **orig, char **extracted) {
  /* strtok_r is MT-safe while strtok is not */
  *extracted = strtok_r(*orig, " ", orig);
  if (*extracted == NULL) {
    return false;
  }
  return true;
}

/**
 * @brief Handle the command
 *
 * This function first extracts one subcommand from the original command, then find one
 * matching name according to the current state (maintained by `ta_state_ptr`).
 *
 * If matched, determine whether the state type is CMD_LAST (to execute function) or
 * CMD_STATE (to step to next state).
 *
 * If not matched, print help message of current state.
 *
 */
static void handle_command(char *str) {
  char *cmd;

  if (extract_one(&str, &cmd)) {
    bool matched = false;
    for (size_t i = 0; i < ta_state_ptr->size; ++i) {
      ta_cli_cmd list_cmd = ta_state_ptr->list[i];
      if (!strncmp(list_cmd.name, cmd, 128)) {
        if (list_cmd.type == CMD_LAST) {
          list_cmd.fn(str);
          matched = true;
          break;
        } else if (list_cmd.type == CMD_STATE) {
          ta_state_ptr = list_cmd.next_state;
          handle_command(str);
          matched = true;
          break;
        }
      }
    }
    if (!matched) do_help("");
  } else {
    do_help("");
  }
}

/**
 * @brief Print help messages
 */
static void do_help(char *cmd) {
  UNUSED(cmd);
  for (size_t i = 0; i < ta_state_ptr->size; ++i) {
    ta_cli_cmd list_cmd = ta_state_ptr->list[i];
    printf("%s: %s", list_cmd.name, list_cmd.describe);

    if (list_cmd.type == CMD_LAST) {
      printf(" (example: %s)", list_cmd.example);
    }
    printf("\n\n");
  }
}

/**
 * @brief Print example usage
 */
static void do_example(char *name) {
  for (size_t i = 0; i < ta_state_ptr->size; ++i) {
    if (!strncmp(ta_state_ptr->list[i].name, name, 128)) {
      printf("Usage: %s\n", ta_state_ptr->list[i].example);
    }
  }
}

static void do_logger_status(char *cmd) {
  UNUSED(cmd);
  printf("%s\n", is_option_enabled(&ta_core->ta_conf, CLI_QUIET_MODE) ? "quiet" : "verbose");
}

static void do_logger_switch(char *cmd) {
  char *selection;
  if (extract_one(&cmd, &selection)) {
    if (!strncmp(selection, "on", 2)) {
      ta_logger_switch(false, false, &ta_core->ta_conf);
      return;
    } else if (!strncmp(selection, "off", 3)) {
      ta_logger_switch(true, false, &ta_core->ta_conf);
      return;
    }
  } else {
    do_example("switch");
  }
}

static void do_redis_memory_set(char *cmd) {
  char *selection;
  if (extract_one(&cmd, &selection)) {
    if (cache_set_capacity(selection) != 1) {
      printf("Cache module not enabled\n");
    }
  } else {
    do_example("set");
  }
}

static void do_redis_memory_get(char *cmd) {
  UNUSED(cmd);
  if (cache_get_capacity() != 1) {
    printf("Cache module not enabled\n");
  }
}
