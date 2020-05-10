#include "accelerator/config.h"
#include "common/ta_errors.h"
#include "config.h"
#include "connectivity/mqtt/duplex_callback.h"
#include "connectivity/mqtt/duplex_utils.h"
#include "connectivity/mqtt/mqtt_common.h"

#define CONN_MQTT_LOGGER "conn-mqtt"

ta_core_t ta_core;
static logger_id_t logger_id;

int main(int argc, char *argv[]) {
  status_t ret;
  mosq_config_t cfg;
  struct mosquitto *mosq = NULL;

  // Initialize logger
  if (ta_logger_init() != SC_OK) {
    return EXIT_FAILURE;
  }

  logger_id = logger_helper_enable(CONN_MQTT_LOGGER, LOGGER_DEBUG, true);

  // Initialize configurations with default value
  if (ta_core_default_init(&ta_core) != SC_OK) {
    return EXIT_FAILURE;
  }

  // Initialize configurations with configuration file
  if (ta_core_file_init(&ta_core, argc, argv) != SC_OK) {
    return EXIT_FAILURE;
  }

  // Initialize configurations with CLI value
  if (ta_core_cli_init(&ta_core, argc, argv) != SC_OK) {
    return EXIT_FAILURE;
  }

  if (ta_core_set(&ta_core) != SC_OK) {
    ta_log_error("Configure failed %s.\n", CONN_MQTT_LOGGER);
    return EXIT_FAILURE;
  }

  // Disable loggers when quiet mode is on
  if (quiet_mode) {
    // Destroy logger when quiet mode is on
    logger_helper_release(logger_id);
    if (logger_helper_destroy() != RC_OK) {
      ta_log_error("Destroying logger failed %s.\n", CONN_MQTT_LOGGER);
      return EXIT_FAILURE;
    }
  } else {
    mqtt_utils_logger_init();
    mqtt_common_logger_init();
    mqtt_callback_logger_init();
    mqtt_pub_logger_init();
    mqtt_sub_logger_init();
    apis_logger_init();
    cc_logger_init();
    pow_logger_init();
    timer_logger_init();
    // Enable backend_redis logger
    br_logger_init();
  }

  // Initialize `mosq` and `cfg`
  // if we want to opertate this program under multi-threading, see https://github.com/eclipse/mosquitto/issues/450
  ret = duplex_config_init(&mosq, &cfg);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  // Set callback functions
  duplex_callback_func_set(mosq);

  // The following one line is used for testing if this server work fine with requests with given topics.
  // Uncomment it if it is necessitated
  // gossip_channel_set(&cfg, MQTT_HOST, "NB/test/room1", "NB/test/room2");

  // Set the configures and message for testing
  ret = gossip_api_channels_set(&cfg, ta_core.ta_conf.mqtt_host, ta_core.ta_conf.mqtt_topic_root);
  if (ret != SC_OK) {
    ta_log_error("%d\n", ret);
    goto done;
  }

  // Set cfg as `userdata` field of `mosq` which allows the callback functions to use `cfg`.
  mosquitto_user_data_set(mosq, &cfg);

  log_info(logger_id, "Starting...\n");

  // Start listening subscribing topics, once we received a message from the listening topics, we can send corresponding
  // message.
  do {
    // TODO Use logger to log some important steps in processing requests.
    log_info(logger_id, "Listening new requests.\n");
    ret = duplex_client_start(mosq, &cfg);
  } while (!ret);

done:
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  mosq_config_free(&cfg);

  if (quiet_mode == false) {
    mqtt_utils_logger_release();
    mqtt_common_logger_release();
    mqtt_callback_logger_release();
    mqtt_pub_logger_release();
    mqtt_sub_logger_release();
    apis_logger_release();
    cc_logger_release();
    serializer_logger_release();
    pow_logger_release();
    timer_logger_release();
    br_logger_release();
    logger_helper_release(logger_id);
    if (logger_helper_destroy() != RC_OK) {
      return EXIT_FAILURE;
    }
  }
  return ret;
}
