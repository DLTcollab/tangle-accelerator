#include "accelerator/config.h"
#include "config.h"
#include "connectivity/mqtt/client_common.h"
#include "connectivity/mqtt/duplex_callback.h"
#include "connectivity/mqtt/duplex_utils.h"
#include "errors.h"

#define CONN_MQTT_LOGGER "conn-mqtt"

ta_core_t ta_core;
static logger_id_t mqtt_logger_id;

int main(int argc, char *argv[]) {
  status_t ret;
  mosq_config_t cfg;
  struct mosquitto *mosq = NULL;

  // Initialize logger
  if (logger_helper_init(LOGGER_DEBUG) != RC_OK) {
    return EXIT_FAILURE;
  }

  mqtt_logger_id = logger_helper_enable(CONN_MQTT_LOGGER, LOGGER_DEBUG, true);

  // Initialize configurations with default value
  if (ta_config_default_init(&ta_core.info, &ta_core.iconf, &ta_core.cache, &ta_core.service) != SC_OK) {
    return EXIT_FAILURE;
  }

  // Initialize configurations with CLI value
  if (ta_config_cli_init(&ta_core, argc, argv) != SC_OK) {
    return EXIT_FAILURE;
  }

  if (ta_config_set(&ta_core.cache, &ta_core.service) != SC_OK) {
    log_critical(mqtt_logger_id, "[%s:%d] Configure failed %s.\n", __func__, __LINE__, CONN_MQTT_LOGGER);
    return EXIT_FAILURE;
  }

  if (verbose_mode) {
    mqtt_utils_logger_init();
    mqtt_common_logger_init();
    mqtt_callback_logger_init();
    mqtt_pub_logger_init();
    mqtt_sub_logger_init();
  } else {
    // Destroy logger when verbose mode is off
    logger_helper_release(mqtt_logger_id);
    logger_helper_destroy();
  }

  // Initialize `mosq` and `cfg`
  // if we want to opertate this program under multi-threading, see https://github.com/eclipse/mosquitto/issues/450
  ret = duplex_config_init(&mosq, &cfg);
  if (ret != SC_OK) {
    log_critical(mqtt_logger_id, "[%s:%d:%d]\n", __func__, __LINE__, ret);
    goto done;
  }

  // Set callback functions
  duplex_callback_func_set(mosq);

  // The following one line is used for testing if this server work fine with requests with given topics.
  // Uncomment it if it is necessitated
  // gossip_channel_set(&cfg, MQTT_HOST, "NB/test/room1", "NB/test/room2");

  // Set the configures and message for testing
  ret = gossip_api_channels_set(&cfg, ta_core.info.mqtt_host, ta_core.info.mqtt_topic_root);
  if (ret != SC_OK) {
    log_critical(mqtt_logger_id, "[%s:%d:%d]\n", __func__, __LINE__, ret);
    goto done;
  }

  // Set cfg as `userdata` field of `mosq` which allows the callback functions to use `cfg`.
  mosquitto_user_data_set(mosq, &cfg);

  log_info(mqtt_logger_id, "Starting...\n");

  // Start listening subscribing topics, once we received a message from the listening topics, we can send corresponding
  // message.
  do {
    // TODO Use logger to log some important steps in processing requests.
    log_info(mqtt_logger_id, "Listening new requests.\n");
    ret = duplex_client_start(mosq, &cfg);
  } while (!ret);

done:
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  mosq_config_free(&cfg);

  if (verbose_mode) {
    mqtt_utils_logger_release();
    mqtt_common_logger_release();
    mqtt_callback_logger_release();
    mqtt_pub_logger_release();
    mqtt_sub_logger_release();
    logger_helper_release(mqtt_logger_id);
    if (logger_helper_destroy() != RC_OK) {
      return EXIT_FAILURE;
    }
  }
  return ret;
}
