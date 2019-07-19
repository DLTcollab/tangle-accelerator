#include "pub_utils.h"
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include "third_party/mosquitto/config.h"

static void set_repeat_time(mosq_config_t *cfg) {
  gettimeofday(&cfg->pub_config->next_publish_tv, NULL);
  cfg->pub_config->next_publish_tv.tv_sec += cfg->pub_config->repeat_delay.tv_sec;
  cfg->pub_config->next_publish_tv.tv_usec += cfg->pub_config->repeat_delay.tv_usec;

  cfg->pub_config->next_publish_tv.tv_sec += cfg->pub_config->next_publish_tv.tv_usec / 1e6;
  cfg->pub_config->next_publish_tv.tv_usec = cfg->pub_config->next_publish_tv.tv_usec % 1000000;
}

static int check_repeat_time(mosq_config_t *cfg) {
  struct timeval tv;
  gettimeofday(&tv, NULL);

  if (tv.tv_sec > cfg->pub_config->next_publish_tv.tv_sec) {
    return EXIT_FAILURE;
  } else if (tv.tv_sec == cfg->pub_config->next_publish_tv.tv_sec &&
             tv.tv_usec > cfg->pub_config->next_publish_tv.tv_usec) {
    return EXIT_FAILURE;
  }
  return 0;
}

mosq_retcode_t publish_message(struct mosquitto *mosq, mosq_config_t *cfg, int *mid, const char *topic, int payloadlen,
                               void *payload, int qos, bool retain) {
  cfg->pub_config->ready_for_repeat = false;
  if (cfg->general_config->protocol_version == MQTT_PROTOCOL_V5 && cfg->pub_config->have_topic_alias &&
      cfg->pub_config->first_publish == false) {
    return mosquitto_publish_v5(mosq, mid, NULL, payloadlen, payload, qos, retain, cfg->property_config->publish_props);
  } else {
    cfg->pub_config->first_publish = false;
    return mosquitto_publish_v5(mosq, mid, topic, payloadlen, payload, qos, retain,
                                cfg->property_config->publish_props);
  }
}

void log_callback_pub_func(struct mosquitto *mosq, void *obj, int level, const char *str) {
  UNUSED(level);

  printf("log: [%s]\n", str);
}

void disconnect_callback_pub_func(struct mosquitto *mosq, void *obj, mosq_retcode_t ret,
                                  const mosquitto_property *properties) {
  UNUSED(ret);
  UNUSED(properties);

  fprintf(stdout, "Publisher disconnect pub callback.\n");
}

void connect_callback_pub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties) {
  mosq_retcode_t ret = MOSQ_ERR_SUCCESS;
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(flags);
  UNUSED(properties);

  if (!result && cfg->pub_config->pub_mode == MSGMODE_CMD) {
    ret = publish_message(mosq, cfg, &cfg->pub_config->mid_sent, cfg->pub_config->topic, cfg->pub_config->msglen,
                          cfg->pub_config->message, cfg->general_config->qos, cfg->general_config->retain);
    if (ret) {
      {
        switch (ret) {
          case MOSQ_ERR_INVAL:
            fprintf(stderr, "Error: Invalid input. Does your topic contain '+' or '#'?\n");
            break;
          case MOSQ_ERR_NOMEM:
            fprintf(stderr, "Error: Out of memory when trying to publish message.\n");
            break;
          case MOSQ_ERR_NO_CONN:
            fprintf(stderr, "Error: Client not connected when trying to publish.\n");
            break;
          case MOSQ_ERR_PROTOCOL:
            fprintf(stderr, "Error: Protocol error when communicating with broker.\n");
            break;
          case MOSQ_ERR_PAYLOAD_SIZE:
            fprintf(stderr, "Error: Message payload is too large.\n");
            break;
          case MOSQ_ERR_QOS_NOT_SUPPORTED:
            fprintf(stderr, "Error: Message QoS not supported on broker, try a lower QoS.\n");
            break;
          default:
            break;
        }
      }
      mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
    }
  } else {
    if (result) {
      if (cfg->general_config->protocol_version == MQTT_PROTOCOL_V5) {
        fprintf(stderr, "%s\n", mosquitto_reason_string(result));
      } else {
        fprintf(stderr, "%s\n", mosquitto_connack_string(result));
      }
    }
  }
  fprintf(stdout, "Publisher connect pub callback.\n");
}

void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties) {
  mosq_config_t *cfg = (mosq_config_t *)obj;
  UNUSED(properties);

  if (reason_code > 127) {
    { fprintf(stderr, "Warning: Publish %d failed: %s.\n", mid, mosquitto_reason_string(reason_code)); }
  }

  if (cfg->pub_config->disconnect_sent == false) {
    mosquitto_disconnect_v5(mosq, 0, cfg->property_config->disconnect_props);
    cfg->pub_config->disconnect_sent = true;
  } else {
    cfg->pub_config->ready_for_repeat = true;
    set_repeat_time(cfg);
  }

  fprintf(stdout, "Publisher publish pub callback.\n");
}

mosq_retcode_t publish_loop(struct mosquitto *mosq, mosq_config_t *cfg) {
  mosq_retcode_t ret = MOSQ_ERR_SUCCESS;
  int loop_delay = 1000;

  if (cfg->pub_config->repeat_delay.tv_sec == 0 || cfg->pub_config->repeat_delay.tv_usec != 0) {
    loop_delay = cfg->pub_config->repeat_delay.tv_usec / 2000;
  }

  mode = cfg->pub_config->pub_mode;

  do {
    ret = mosquitto_loop(mosq, loop_delay, 1);
    if (cfg->pub_config->ready_for_repeat && check_repeat_time(cfg)) {
      ret = MOSQ_ERR_SUCCESS;
      switch (cfg->pub_config->pub_mode) {
        case MSGMODE_CMD:
          ret = publish_message(mosq, cfg, &cfg->pub_config->mid_sent, cfg->pub_config->topic, cfg->pub_config->msglen,
                                cfg->pub_config->message, cfg->general_config->qos, cfg->general_config->retain);
          break;
      }
      if (ret) {
        fprintf(stderr, "Error sending repeat publish: %s", mosquitto_strerror(ret));
        return ret;
      }
    }
  } while (ret == MOSQ_ERR_SUCCESS);
  return MOSQ_ERR_SUCCESS;
}

mosq_retcode_t init_check_error(mosq_config_t *cfg, client_type_t client_type) {
  rc_mosq_retcode_t ret = RC_MOS_OK;

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
    return EXIT_FAILURE;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
    return EXIT_FAILURE;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    fprintf(stderr, "Warning: Not using password since username not set.\n");
  }

  if (cfg->general_config->will_payload && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will payload given, but no will topic given.\n");
    return EXIT_FAILURE;
  }
  if (cfg->general_config->will_retain && !cfg->general_config->will_topic) {
    fprintf(stderr, "Error: Will retain given, but no will topic given.\n");
    return EXIT_FAILURE;
  }
  if (cfg->general_config->password && !cfg->general_config->username) {
    fprintf(stderr, "Warning: Not using password since username not set.\n");
  }
#ifdef WITH_TLS
  if ((cfg->tls_config->certfile && !cfg->tls_config->keyfile) ||
      (cfg->tls_config->keyfile && !cfg->tls_config->certfile)) {
    fprintf(stderr, "Error: Both certfile and keyfile must be provided if one of them is set.\n");
    return EXIT_FAILURE;
  }
  if ((cfg->tls_config->keyform && !cfg->tls_config->keyfile)) {
    fprintf(stderr, "Error: If keyform is set, keyfile must be also specified.\n");
    return EXIT_FAILURE;
  }
  if ((cfg->tls_config->tls_engine_kpass_sha1 && (!cfg->tls_config->keyform || !cfg->tls_config->tls_engine))) {
    fprintf(stderr, "Error: when using tls-engine-kpass-sha1, both tls-engine and keyform must also be provided.\n");
    return EXIT_FAILURE;
  }
#endif
#ifdef FINAL_WITH_TLS_PSK
  if ((cfg->tls_config->cafile || cfg->tls_config->capath) && cfg->tls_config->psk) {
    fprintf(stderr, "Error: Only one of --psk or --cafile/--capath may be used at once.\n");
    return EXIT_FAILURE;
  }
  if (cfg->tls_config->psk && !cfg->tls_config->psk_identity) {
    fprintf(stderr, "Error: --psk-identity required if --psk used.\n");
    return EXIT_FAILURE;
  }
#endif

  if (cfg->general_config->clean_session == false && (cfg->general_config->id_prefix || !cfg->general_config->id)) {
    fprintf(stderr, "Error: You must provide a client id if you are using the -c option.\n");
    return EXIT_FAILURE;
  }

  if (client_type == client_sub) {
    if (cfg->sub_config->topic_count == 0) {
      fprintf(stderr, "Error: You must specify a topic to subscribe to.\n");
      return EXIT_FAILURE;
    }
  }

  if (!cfg->general_config->host) {
    cfg->general_config->host = strdup("localhost");
    if (!cfg->general_config->host) {
      fprintf(stderr, "Error: Out of memory.\n");
      return EXIT_FAILURE;
    }
  }

  ret = mosquitto_property_check_all(CMD_CONNECT, cfg->property_config->connect_props);
  if (ret) {
    fprintf(stderr, "Error in CONNECT properties: %s\n", mosquitto_strerror(ret));
    return EXIT_FAILURE;
  }
  ret = mosquitto_property_check_all(CMD_PUBLISH, cfg->property_config->publish_props);
  if (ret) {
    fprintf(stderr, "Error in PUBLISH properties: %s\n", mosquitto_strerror(ret));
    return EXIT_FAILURE;
  }
  ret = mosquitto_property_check_all(CMD_SUBSCRIBE, cfg->property_config->subscribe_props);
  if (ret) {
    fprintf(stderr, "Error in SUBSCRIBE properties: %s\n", mosquitto_strerror(ret));
    return EXIT_FAILURE;
  }
  ret = mosquitto_property_check_all(CMD_UNSUBSCRIBE, cfg->property_config->unsubscribe_props);
  if (ret) {
    fprintf(stderr, "Error in UNSUBSCRIBE properties: %s\n", mosquitto_strerror(ret));
    return EXIT_FAILURE;
  }
  ret = mosquitto_property_check_all(CMD_DISCONNECT, cfg->property_config->disconnect_props);
  if (ret) {
    fprintf(stderr, "Error in DISCONNECT properties: %s\n", mosquitto_strerror(ret));
    return EXIT_FAILURE;
  }
  ret = mosquitto_property_check_all(CMD_WILL, cfg->property_config->will_props);
  if (ret) {
    fprintf(stderr, "Error in Will properties: %s\n", mosquitto_strerror(ret));
    return EXIT_FAILURE;
  }

  return MOSQ_ERR_SUCCESS;
}
