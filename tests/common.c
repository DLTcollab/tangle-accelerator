#include "common.h"
#include "accelerator/cli_info.h"
#include "stdlib.h"

static struct ta_cli_argument_s driver_cli_arguments_g[] = {
    {"hash", required_argument, NULL, 'H', "testing transaction hashes"},
    {"tag", required_argument, NULL, 'T', "testing transaction tag"},
    {NULL, 0, NULL, 0, NULL}};

struct option* driver_cli_build_options() {
  int driver_cli_arg_size = sizeof(driver_cli_arguments_g) / sizeof(driver_cli_arguments_g[0]);
  struct option* driver_long_options =
      (struct option*)realloc(cli_build_options(), (cli_cmd_num + 2) * sizeof(struct option));
  for (int i = 0; i < driver_cli_arg_size; i++) {
    driver_long_options[(cli_cmd_num - 1) + i].name = driver_cli_arguments_g[i].name;
    driver_long_options[(cli_cmd_num - 1) + i].has_arg = driver_cli_arguments_g[i].has_arg;
    driver_long_options[(cli_cmd_num - 1) + i].flag = driver_cli_arguments_g[i].flag;
    driver_long_options[(cli_cmd_num - 1) + i].val = driver_cli_arguments_g[i].val;
  }

  return driver_long_options;
}

status_t driver_core_cli_init(ta_core_t* const core, int argc, char** argv, driver_test_cases_t* test_cases) {
  int key = 0;
  status_t ret = SC_OK;
  struct option* long_options = driver_cli_build_options();

  while ((key = getopt_long(argc, argv, "hvH:T:", long_options, NULL)) != -1) {
    switch (key) {
      case 'h':
        ta_usage();
        exit(EXIT_SUCCESS);
      case 'v':
        printf("%s\n", TA_VERSION);
        exit(EXIT_SUCCESS);
      case 'H':
        // Take the arguments as testing transaction hashes
        if (test_cases) {
          for (int i = 0; i < TXN_HASH_NUM; i++) {
            if (!test_cases->txn_hash[i]) {
              test_cases->txn_hash[i] = optarg;
            }
          }
        }
        break;
      case 'T':
        // Take the arguments as testing transaction tag
        if (test_cases) {
          test_cases->tag = optarg;
        }
        break;
      default:
        ret = cli_core_set(core, key, optarg);
        break;
    }
    if (ret != SC_OK) {
      break;
    }
  }

  free(long_options);
  return ret;
}

static double diff_time(struct timespec start, struct timespec end) {
  struct timespec diff;
  if (end.tv_nsec - start.tv_nsec < 0) {
    diff.tv_sec = end.tv_sec - start.tv_sec - 1;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec + 1000000000;
  } else {
    diff.tv_sec = end.tv_sec - start.tv_sec;
    diff.tv_nsec = end.tv_nsec - start.tv_nsec;
  }
  return (diff.tv_sec + diff.tv_nsec / 1000000000.0);
}

void test_time_end(struct timespec* start, struct timespec* end, double* sum) {
  clock_gettime(CLOCK_REALTIME, end);
  double difference = diff_time(*start, *end);
#if defined(ENABLE_STAT)
  printf("%lf\n", difference);
#endif
  *sum += difference;
}

void rand_trytes_init() {
  // We use ASLR which stands for Address Space Layout Randomization, and we can assume each address of functions inside
  // loaded program is randomized
  srand(getpid() ^ ((unsigned long)(&rand_trytes_init)));
}
