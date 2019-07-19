#ifndef PUB_UTILS_H
#define PUB_UTILS_H

#include "client_common.h"
#include "third_party/mosquitto/lib/mosquitto.h"
#include "third_party/mosquitto/lib/mqtt_protocol.h"

extern int mid_sent;

void log_callback_pub_func(struct mosquitto *mosq, void *obj, int level, const char *str);
void disconnect_callback_pub_func(struct mosquitto *mosq, void *obj, mosq_retcode_t ret,
                                  const mosquitto_property *properties);
void connect_callback_pub_func(struct mosquitto *mosq, void *obj, int result, int flags,
                               const mosquitto_property *properties);
void publish_callback_pub_func(struct mosquitto *mosq, void *obj, int mid, int reason_code,
                               const mosquitto_property *properties);
mosq_retcode_t publish_message(struct mosquitto *mosq, mosq_config_t *cfg, int *mid, const char *topic, int payloadlen,
                               void *payload, int qos, bool retain);
mosq_retcode_t publish_loop(struct mosquitto *mosq, mosq_config_t *cfg);
mosq_retcode_t init_check_error(mosq_config_t *cfg, client_type_t client_type);

#endif
