#ifndef ACCELERATOR_MESSAGE_H_
#define ACCELERATOR_MESSAGE_H_

#ifdef __cplusplus
extern "C" {
#endif

typedef enum ta_cli_arg_value_e {
  HELP_CLI,

  /** TA */
  TA_HOST_CLI,
  TA_PORT_CLI,
  TA_THREAD_COUNT_CLI,
  TA_VERSION_CLI,

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
} ta_cli_arg_value_t;

typedef enum ta_cli_arg_requirement_e {
  NO_ARG,
  REQUIRED_ARG,
  OPTIONAL_ARG
} ta_cli_arg_requirement_t;

static struct ta_cli_argument_s {
  char* name;
  int val;
  char* desc;
  ta_cli_arg_requirement_t has_arg;
} ta_cli_arguments_g[] = {
    {"ta-help", HELP_CLI, "Show tangle-accelerator usage.", NO_ARG}};

static const int cli_cmd_num =
    sizeof(ta_cli_arguments_g) / sizeof(struct ta_cli_argument_s);
void ta_usage();
#ifdef __cplusplus
}
#endif

#endif  // ACCELERATOR_MESSAGE_H_
