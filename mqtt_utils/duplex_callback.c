#include "duplex_callback.h"
#include "duplex_utils.h"
#include "pub_utils.h"
#include "sub_utils.h"

static void publish_callback_duplex_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                                         const mosquitto_property *properties) {
  publish_callback_pub_func(mosq, obj, mid, reason_code, properties);
  printf("publish_callback_duplex_func \n");
}

static void message_callback_duplex_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                                         const mosquitto_property *properties) {
  message_callback_sub_func(mosq, obj, message, properties);
  mosquitto_disconnect_v5(mosq, 0, ((mosq_config_t *)obj)->property_config->disconnect_props);
  printf("message_callback_duplex_func \n");
}

static void connect_callback_duplex_func(struct mosquitto *mosq, void *obj, int result, int flags,
                                         const mosquitto_property *properties) {
  if (((mosq_config_t *)obj)->general_config->client_type == client_pub) {
    connect_callback_pub_func(mosq, obj, result, flags, properties);
  } else if (((mosq_config_t *)obj)->general_config->client_type == client_sub) {
    connect_callback_sub_func(mosq, obj, result, flags, properties);
  }

  printf("connect_callback_duplex_func \n");
}

static void disconnect_callback_duplex_func(struct mosquitto *mosq, void *obj, mosq_retcode_t ret,
                                            const mosquitto_property *properties) {
  if (((mosq_config_t *)obj)->general_config->client_type == client_pub) {
    disconnect_callback_pub_func(mosq, obj, ret, properties);
  } else if (((mosq_config_t *)obj)->general_config->client_type == client_sub) {
  }

  fprintf(stdout, "disconnect_callback_duplex_func \n");
}

static void subscribe_callback_duplex_func(struct mosquitto *mosq, void *obj, int mid, int qos_count,
                                           const int *granted_qos) {
  subscribe_callback_sub_func(mosq, obj, mid, qos_count, granted_qos);
  printf("subscribe_callback_duplex_func \n");
}

static void log_callback_duplex_func(struct mosquitto *mosq, void *obj, int level, const char *str) {
  printf("%s\n", str);
}

rc_mosq_retcode_t duplex_callback_func_set(struct mosquitto *mosq, mosq_config_t *cfg) {
  if (cfg->general_config->debug) {
    mosquitto_log_callback_set(mosq, log_callback_duplex_func);
    mosquitto_subscribe_callback_set(mosq, subscribe_callback_duplex_func);
  }
  mosquitto_connect_v5_callback_set(mosq, connect_callback_duplex_func);
  mosquitto_disconnect_v5_callback_set(mosq, disconnect_callback_duplex_func);
  mosquitto_publish_v5_callback_set(mosq, publish_callback_duplex_func);
  mosquitto_message_v5_callback_set(mosq, message_callback_duplex_func);
}