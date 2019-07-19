#include "mqtt_utils/client_common.h"
#include "mqtt_utils/duplex_callback.h"
#include "mqtt_utils/duplex_utils.h"

int main(int argc, char *argv[]) {
  rc_mosq_retcode_t ret;
  mosq_config_t cfg;
  struct mosquitto *mosq = NULL;

  // Initialize `mosq` and `cfg`
  // if we want to opertate this program under multi-threading, see https://github.com/eclipse/mosquitto/issues/450
  duplex_config_init(&mosq, &cfg);

  // Set callback functions
  ret = duplex_callback_func_set(mosq, &cfg);
  if (ret) {
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
    goto done;
  }

  // Set the configures and message for testing
  ret = gossip_channel_set(&cfg, HOST, TOPIC, TOPIC_RES);
  if (ret) {
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
    goto done;
  }

  // Set the message that is going to be sent. This function could be used in the function `duplex_loop`
  // We just put it here for demostration.
  ret = gossip_message_set(&cfg, MESSAGE);
  if (ret) {
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
    goto done;
  }

  // Set cfg as `userdata` field of `mosq` which allows the callback functions to use `cfg`.
  mosquitto_user_data_set(mosq, &cfg);

  // Start listening subscribing topics, once we received a message from the listening topics, we can send corresponding
  // message.
  // if we need to take the above task forever, just put it in a infinit loop.
  ret = duplex_loop(mosq, &cfg);
  if (ret) {
    fprintf(stderr, "Error: %s\n", mosquitto_strerror(ret));
  }

done:
  mosquitto_destroy(mosq);
  mosquitto_lib_cleanup();
  mosq_config_cleanup(&cfg);
  return ret;
}
