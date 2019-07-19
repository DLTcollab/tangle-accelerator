#include "sub_utils.h"
#include <signal.h>
#include "third_party/mosquitto/config.h"

static void write_payload(const unsigned char *payload, int payloadlen, int hex) {
  if (hex == 0) {
    (void)fwrite(payload, 1, payloadlen, stdout);
  } else if (hex == 1 || hex == 2) {
    for (int i = 0; i < payloadlen; i++) {
      fprintf(stdout, "%02x", payload[i]);
    }
  }
}

static void print_message(mosq_config_t *cfg, const struct mosquitto_message *message) {
  if (message->payloadlen) {
    write_payload(message->payload, message->payloadlen, false);
    printf("\n");
    fflush(stdout);
  }
}

void signal_handler_func(int signum) {
  if (signum == SIGALRM) {
    // TODO: Find some way else to break a loop or disconnect the client, in which way we don't
    // need to use variable `mosq`, `cfg`
    // mosquitto_disconnect_v5(mosq, MQTT_RC_DISCONNECT_WITH_WILL_MSG, cfg.property_config->disconnect_props);
  }
}

void publish_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(reason_code);
  UNUSED(properties);

  if ((mid == cfg->general_config->last_mid || cfg->general_config->last_mid == 0)) {
    mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
  }

  printf("publish_callback_sub_func \n");
}

void message_callback_sub_func(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message,
                               const mosquitto_property *properties) {
  bool res;
  mosq_config_t *cfg = (mosq_config_t *)obj;

  if (cfg->sub_config->remove_retained && message->retain) {
    mosquitto_publish(mosq, &cfg->general_config->last_mid, message->topic, 0, NULL, 1, true);
  }

  if (cfg->sub_config->retained_only && !message->retain) {
    if (cfg->general_config->last_mid == 0) {
      mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
    }
    return;
  }

  if (message->retain && cfg->sub_config->no_retain) return;
  if (cfg->sub_config->filter_outs) {
    for (int i = 0; i < cfg->sub_config->filter_out_count; i++) {
      mosquitto_topic_matches_sub(cfg->sub_config->filter_outs[i], message->topic, &res);
      if (res) return;
    }
  }

  print_message(cfg, message);

  // Uncomment the following code would cause: once we received a message, then disconnect the connection.
  // mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);

  printf("message_callback_sub_func \n");
}

void connect_callback_sub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;

  UNUSED(flags);
  UNUSED(properties);

  if (!result) {
    mosquitto_subscribe_multiple(mosq, NULL, cfg->sub_config->topic_count, cfg->sub_config->topics,
                                 cfg->general_config->qos, cfg->sub_config->sub_opts,
                                 cfg->property_config->subscribe_props);

    for (int i = 0; i < cfg->sub_config->unsub_topic_count; i++) {
      mosquitto_unsubscribe_v5(mosq, NULL, cfg->sub_config->unsub_topics[i], cfg->property_config->unsubscribe_props);
    }
  } else {
    if (result) {
      if (cfg->general_config->protocol_version == MQTT_PROTOCOL_V5) {
        fprintf(stderr, "%s\n", mosquitto_reason_string(result));
      } else {
        fprintf(stderr, "%s\n", mosquitto_connack_string(result));
      }
    }
    mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
  }

  printf("connect_callback_sub_func \n");
}

void subscribe_callback_sub_func(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos) {
  mosq_config_t *cfg = (mosq_config_t *)obj;

  printf("Subscribed (mid: %d): %d", mid, granted_qos[0]);
  for (int i = 1; i < qos_count; i++) {
    printf(", %d", granted_qos[i]);
  }
  printf("\n");

  if (cfg->sub_config->exit_after_sub) {
    mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
  }

  printf("subscribe_callback_sub_func \n");
}

void log_callback_sub_func(struct mosquitto *mosq, void *obj, int level, const char *str) {
  UNUSED(mosq);
  UNUSED(level);

  printf("%s\n", str);
}